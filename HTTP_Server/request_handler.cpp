//
// Created by programajor on 11/16/18.
//

#include <strings.h>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <iostream>
#include <sstream>
#include <fstream>
#include <iostream>
#include <algorithm>
#include "request_handler.h"
#include "server_constants.h"
#include "request_parser.h"

using namespace std;

void initialize_file_extensions_if_needed();
map<string, string> get_headers_map(string header);

map<string, string> FILE_EXTENSIONS;
map<string, string> CONTENT_TO_FILE_EXTENSIONS;

char* append(const char *s, const char* c, long lenS) {
    size_t lenC = SERVER_BUFFER_SIZE;
    char buf[lenS + lenC];
    memcpy(buf, s, lenS);
    memcpy(buf + lenS, c, lenC);

    return strdup(buf);
}

void handle_request(int client_fd) {
    bool connection_close = false;
    char buffer[SERVER_BUFFER_SIZE];
    unsigned long s = string::npos;
    ssize_t n;
    char* total = new char();
    string totalString;
    long read_in_buffer = 0;
    do {
        bzero(buffer, SERVER_BUFFER_SIZE);
        n = read(client_fd, buffer, SERVER_BUFFER_SIZE - 1);
        if (n == 0) return;
        if (n < 0) {
            cout << "ERROR reading from socket\n";
            exit(1);
        }
        total = append(total, buffer, read_in_buffer);
        read_in_buffer += n;
        totalString = total;
        s = totalString.find(REQUEST_HEADER_END);
    } while (s == string::npos);
    while (true) {
        while (s == string::npos) {
            bzero(buffer, SERVER_BUFFER_SIZE);
            n = read(client_fd, buffer, SERVER_BUFFER_SIZE - 1);
            if (n == 0) return;
            if (n < 0) {
                cout << "ERROR reading from socket\n";
                exit(1);
            }
            total = append(total, buffer, read_in_buffer);
            read_in_buffer += n;
            totalString = total;
            s = totalString.find(REQUEST_HEADER_END);
        }

        string header = totalString.substr(0, s);
        char* body = &total[s + 4];
        struct server_request req = extract_request_params_from_header(header);
        map<string, string> headersMap = get_headers_map(header);
        if (headersMap["Connection"] == "close" || headersMap["Connection"] == "close\\r") connection_close = true;
        if (req.request_type == GET) {
            read_in_buffer -= (header.length() + 4);
            // handle get from header only and continue reading other requests
            string file_name = req.file_name.substr(1);
            ifstream inFile;
            inFile.open(file_name, ios::binary);
            if (!inFile) {
                inFile.close();
                const char *response = "HTTP/1.1 404 Not Found\r\n\r\n";
                ssize_t n = write(client_fd, response, strlen(response));
                if (n < 0) {
                    cout << "ERROR writing to socket\n";
                    exit(1);
                }
            } else {
                inFile.seekg(0, ios::end);
                long len = inFile.tellg();
                inFile.seekg(0, ios::beg);
                char *file_body = new char[len];
                inFile.read(file_body, len);
                inFile.close();

                vector<string> tokens = split(file_name, '.');
                if (tokens.size() == 1) tokens.emplace_back("");
                initialize_file_extensions_if_needed();
                string content_type = FILE_EXTENSIONS[tokens[1]];

                string headers = "HTTP/1.1 200 OK\r\nContent-Type: " + content_type
                                 + "\r\nContent-Length: " + to_string(len) + "\r\n\r\n";
                ssize_t n = write(client_fd, headers.c_str(), strlen(headers.c_str()));
                write(client_fd, file_body, (int) len);
                if (n < 0) {
                    cout << "ERROR writing to socket\n";
                    exit(1);
                }
            }
            if (connection_close) {
                while (n > 0) {
                    n = read(client_fd, buffer, SERVER_BUFFER_SIZE - 1);
                }
                return;
            }
            total = body;
            totalString = total;
            s = totalString.find(REQUEST_HEADER_END);
        } else if (req.request_type == POST) {
            // keep reading from socket till reaching content length
            // then move to next requests
            const char *response = "HTTP/1.1 200 OK\r\n\r\n";
            n = write(client_fd, response, strlen(response));
            if (n < 0) {
                cout << "ERROR writing to socket\n";
                exit(1);
            }
            int content_length = atoi(headersMap["Content-Length"].c_str());

            // get file name
            string file_name = req.file_name.substr(1);
            vector<string> tokens = split(file_name, '/');
            string name = tokens[tokens.size()-1];
            unsigned long s3 = name.find('.');
            if (s3 == string::npos) { // didn't find extension in the file name
                initialize_file_extensions_if_needed();
                string content_type = headersMap["Content-Type"];
                content_type = content_type.substr(0, content_type.size() - 1);
                name += "." + CONTENT_TO_FILE_EXTENSIONS[content_type];
            }
            file_name = "";
            for (int i = 0; i < tokens.size() - 1; i++) {
                file_name += tokens[i] + '/';
            }
            file_name += name;

            bool first_time_write = true;
            total = body;
            while ((read_in_buffer - (header.length() + 4) <= content_length) || (!first_time_write && read_in_buffer <= content_length)) {
                // request not ended yet
                if (first_time_write) {
                    FILE *file_to_save = fopen(file_name.c_str(), "w+");
                    fwrite((void*) (total), sizeof(char), sizeof(char) * ((int) read_in_buffer - (header.length() + 4)), file_to_save);
                    fclose(file_to_save);
                } else {
                    FILE *file_to_save = fopen(file_name.c_str(), "a");
                    fwrite((void*) (total), sizeof(char), sizeof(char) * ((int) read_in_buffer), file_to_save);
                    fclose(file_to_save);
                }
                if (first_time_write) {
                    content_length -= read_in_buffer - (header.length() + 4);
                } else {
                    content_length -= read_in_buffer;
                }
                // read again
                bzero(buffer, SERVER_BUFFER_SIZE);
                n = read(client_fd, buffer, SERVER_BUFFER_SIZE - 1);
                if (n == 0) return;
                if (n < 0) {
                    cout << "ERROR reading from socket\n";
                    exit(1);
                }
                read_in_buffer = n;
                total = buffer;
                first_time_write = false;
            }
            // total > content_length
            if (first_time_write) {
                FILE *file_to_save = fopen(file_name.c_str(), "w+");
                fwrite((void*) (total), sizeof(char), sizeof(char) * (content_length), file_to_save);
                fclose(file_to_save);
            } else {
                FILE *file_to_save = fopen(file_name.c_str(), "a");
                fwrite((void*) (total), sizeof(char), sizeof(char) * (content_length), file_to_save);
                fclose(file_to_save);
            }
            if (connection_close) {
                while (n > 0) {
                    n = read(client_fd, buffer, SERVER_BUFFER_SIZE - 1);
                }
                return;
            }
            char* newTotal = new char();
            read_in_buffer -= content_length;
            char* temp = &total[content_length];
            memcpy(newTotal, temp, read_in_buffer);
            total = newTotal;
            total[read_in_buffer] = '\0';
            totalString = temp;
            s = totalString.find(REQUEST_HEADER_END);
        }
    }
}

map<string, string> get_headers_map(string header) {
    stringstream ss(header);
    string line;
    map<string, string> ret;
    while (getline(ss, line)) {
        unsigned long s = line.find(": ");
        if (s != string::npos) {
            ret[line.substr(0, s)] = line.substr(s+2, line.size()-1);
        }
    }
    return ret;
}

void initialize_file_extensions_if_needed() {
    if (FILE_EXTENSIONS.size() == 0) {
        FILE_EXTENSIONS["jpg"] = "image/jpeg";
        FILE_EXTENSIONS["png"] = "image/png";
        FILE_EXTENSIONS["html"] = "text/html";
        FILE_EXTENSIONS["txt"] = "text/plain";
        FILE_EXTENSIONS[""] = "text/plain";
        CONTENT_TO_FILE_EXTENSIONS["image/jpeg"] = "jpg";
        CONTENT_TO_FILE_EXTENSIONS["image/png"] = "png";
        CONTENT_TO_FILE_EXTENSIONS["text/html"] = "html";
        CONTENT_TO_FILE_EXTENSIONS["text/plain"] = "txt";
        CONTENT_TO_FILE_EXTENSIONS[""] = "txt";
    }
}
