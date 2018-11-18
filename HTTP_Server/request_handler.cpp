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

string get_file_name(const server_request &req, map<string, string> &headersMap);

string get_response_headers(const string &file_name, long len);

void get_new_total(int content_length, char *&total, string &totalString, long &read_in_buffer);

map<string, string> FILE_EXTENSIONS;
map<string, string> CONTENT_TO_FILE_EXTENSIONS;

char* append(const char *s, const char* c, long lenS, long lenC) {
    char buf[lenS + lenC];
    memcpy(buf, s, lenS);
    memcpy(buf + lenS, c, lenC);
    return strdup(buf);
}

bool last_request(map<string, string> headersMap) {
    return headersMap["Connection"] == "close" || headersMap["Connection"] == "close\\r";
}

void process_request_from_header(struct server_request req, char* buffer,  long remaining_content_length, int n, int start,
        FILE **file_to_save, map<string, string> header_map) {
    int length = n < remaining_content_length ? n : remaining_content_length;

    if (req.request_type == GET) {
        // process all GET
        string file_name = req.file_name.substr(1);
        ifstream inFile;
        inFile.open(file_name, ios::binary);
        if (!inFile) {
            inFile.close();
            const char *response = "HTTP/1.1 404 Not Found\r\n\r\n";
            ssize_t n = write(req.client_fd, response, strlen(response));
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

            string headers = get_response_headers(file_name, len);
            n = write(req.client_fd, headers.c_str(), strlen(headers.c_str()));
            write(req.client_fd, file_body, (int) len);
            if (n < 0) {
                cout << "ERROR writing to socket\n";
                exit(1);
            }
        }
    } else if (req.request_type == POST) {
        const char *response = "HTTP/1.1 200 OK\r\n\r\n";
        write(req.client_fd, response, strlen(response));
        *file_to_save = fopen(get_file_name(req, header_map).c_str(), "w+");
        fwrite((void *) &buffer[start], sizeof(char), sizeof(char) * length, *file_to_save);
    }
}

void handle_request(int client_fd) {
    cout << "New connection created " << client_fd << ".\n";

    bool end_connection = false;
    bool connection_close = false; // marked true when request header contains Connection: close
    char current_buffer[SERVER_BUFFER_SIZE];
    ssize_t n = 1;
    char* buffer = "";
    long buffer_size = 0;
    bool post_request = false;
    long remaining_content_length = 0;
    struct server_request req;
    FILE *file_to_save = nullptr;
    bool has_previous_data = false;
    do {
        bzero(current_buffer, SERVER_BUFFER_SIZE);
        if (has_previous_data) {
            has_previous_data = false;
            n = buffer_size;
            memcpy(current_buffer, buffer, buffer_size);
            buffer = "";
            buffer_size = 0;
        } else {
            n = read(client_fd, current_buffer, SERVER_BUFFER_SIZE - 1);
        }

        if (n <= 0) {
            if (n < 0) cerr << "Error with reading data from socket\n";
            end_connection = true;
        } else {
            buffer = append(buffer, current_buffer, buffer_size, n);
            string current_data_as_string = buffer;
            unsigned long s = current_data_as_string.find(REQUEST_HEADER_END);
            if (post_request) {
                int length = n < remaining_content_length ? n : remaining_content_length;
                fwrite((void *) current_buffer, sizeof(char), sizeof(char) * length, file_to_save);
                remaining_content_length -= n;
            } else if (s != string::npos) { // finish reading header data
                string header = current_data_as_string.substr(0, s);
                buffer = "";
                map<string, string> header_map = get_headers_map(header);
                if (last_request(header_map)) connection_close = true;
                req = extract_request_params_from_header(header);
                req.client_fd = client_fd;
                // TODO file hirerachy
                remaining_content_length = atol(header_map["Content-Length"].c_str());
                int readed_body_length = n - s - 4 > remaining_content_length ? remaining_content_length : n - s - 4;
                remaining_content_length -= (n - s - 4);
                process_request_from_header(req, current_buffer, readed_body_length, n, s+4, &file_to_save, header_map);
                if (req.request_type == POST && remaining_content_length != 0) post_request = true;
            } else { // header not ended
                buffer = append(buffer, current_buffer, buffer_size, n);
                buffer_size += n ;
            }

            if (remaining_content_length == 0 && req.request_type == POST) fclose(file_to_save);
            else if (remaining_content_length < 0) {
                // copy new request values to buffer
                int new_request_buffer_size = remaining_content_length * -1;
                buffer_size = new_request_buffer_size;
                buffer = new char[new_request_buffer_size];
                memcpy(buffer, current_buffer + n - new_request_buffer_size, new_request_buffer_size);

                // resetting all parameters
                if (req.request_type == POST) fclose(file_to_save);
                post_request = false;
                remaining_content_length = 0;

                has_previous_data = true;
            }

            if (connection_close && (req.request_type == GET || remaining_content_length == 0)) end_connection = true;
        }

    } while (!end_connection);

    close(client_fd);
    cout << "Closing connection " << client_fd << ".\n";
}


void get_new_total(int content_length, char *&total, string &totalString, long &read_in_buffer) {
    char* newTotal = new char();
    read_in_buffer -= content_length;
    char* temp = &total[content_length];
    memcpy(newTotal, temp, read_in_buffer);
    total = newTotal;
    total[read_in_buffer] = '\0';
    totalString = temp;
}

string get_response_headers(const string &file_name, long len) {
    vector<string> tokens = split(file_name, '.');
    if (tokens.size() == 1) tokens.emplace_back("");
    initialize_file_extensions_if_needed();
    string content_type = FILE_EXTENSIONS[tokens[1]];
    string headers = "HTTP/1.1 200 OK\r\nContent-Type: " + content_type
                                 + "\r\nContent-Length: " + to_string(len) + "\r\n\r\n";
    return headers;
}

string get_file_name(const server_request &req, map<string, string> &headersMap) {
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
    return file_name;
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
