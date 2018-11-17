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
string get_file_name(const server_request &request, map<string,string> headersMap);
void handle_get_request(server_request request, int client_fd);
void handle_post_request(server_request request, int client_fd, const char* body, string header);
void save_content(string file_name, const char *content, const ssize_t len, string mode);

map<string, string> FILE_EXTENSIONS;
map<string, string> CONTENT_TO_FILE_EXTENSIONS;

void handle_request(int client_fd) {
    char buffer[SERVER_BUFFER_SIZE];
    unsigned long s = string::npos;
    ssize_t n;
    string total;
    do {
        bzero(buffer, SERVER_BUFFER_SIZE);
        n = read(client_fd, buffer, SERVER_BUFFER_SIZE - 1);
        if (n < 0) {
            cout << "ERROR reading from socket\n";
            exit(1);
        }
        total += buffer;
        s = total.find(REQUEST_HEADER_END);
    } while (s == string::npos);
    while (true) {
        while (s == string::npos) {
            bzero(buffer, SERVER_BUFFER_SIZE);
            n = read(client_fd, buffer, SERVER_BUFFER_SIZE - 1);
            if (n < 0) {
                cout << "ERROR reading from socket\n";
                exit(1);
            }
            total += buffer;
            s = total.find(REQUEST_HEADER_END);
        }

        string header = total.substr(0, s);
        string body = total.substr(s + 4);
        struct server_request req = extract_request_params_from_header(header);
        map<string, string> headersMap = get_headers_map(header);
        if (req.request_type == GET) {
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
            total = body;
            s = total.find(REQUEST_HEADER_END);
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
            while (total.length() <= content_length) {
//                char* tmp = buffer;
//                unsigned long s4 = tmp.find(REQUEST_HEADER_END);
                // request not ended yet
                if (first_time_write) {
                    FILE *file_to_save = fopen(file_name.c_str(), "w+");
                    fwrite((void*) (total.c_str()), sizeof(char), sizeof(char) * ((int) total.size()), file_to_save);
                    fclose(file_to_save);
                } else {
                    FILE *file_to_save = fopen(file_name.c_str(), "a");
                    fwrite((void*) (total.c_str()), sizeof(char), sizeof(char) * ((int) total.size()), file_to_save);
                    fclose(file_to_save);
                }
                content_length -= total.length();
                // read again
                bzero(buffer, SERVER_BUFFER_SIZE);
                n = read(client_fd, buffer, SERVER_BUFFER_SIZE - 1);
                if (n < 0) {
                    cout << "ERROR reading from socket\n";
                    exit(1);
                }
                total = buffer;
                first_time_write = false;
            }
            // total > content_length
            body = total.substr(0, content_length);
            if (first_time_write) {
                FILE *file_to_save = fopen(file_name.c_str(), "w+");
                fwrite((void*) (body.c_str()), sizeof(char), sizeof(char) * ((int) body.size()), file_to_save);
                fclose(file_to_save);
            } else {
                FILE *file_to_save = fopen(file_name.c_str(), "a");
                fwrite((void*) (body.c_str()), sizeof(char), sizeof(char) * ((int) body.size()), file_to_save);
                fclose(file_to_save);
            }
            total = total.substr(content_length);
            content_length -= body.length();
            s = total.find(REQUEST_HEADER_END);
        }
    }
}

//void handle_request(int client_fd) {
//    char buffer[SERVER_BUFFER_SIZE];
//    bzero(buffer, SERVER_BUFFER_SIZE);
//    bool headerEnded = false;
//    string header = "";
//    string totalReadTillNow = "";
//    ssize_t n = read(client_fd, buffer, SERVER_BUFFER_SIZE - 1);
//
//    if (n < 0) {
//        cout << "ERROR reading from socket\n";
//        exit(1);
//    }
//
//    while (n > 0) {
//        totalReadTillNow += buffer;
//        unsigned long s = totalReadTillNow.find(REQUEST_HEADER_END);
//
//        if (headerEnded) { // appending body data to file
//
//        } else if (s != string::npos) { // finish reading header data
//            header = totalReadTillNow.substr(0, s);
//            cout << totalReadTillNow;
//            struct server_request req = extract_request_params_from_header(header);
//            if (req.request_type == GET) {
//                handle_get_request(req, client_fd);
//                header = "";
//                totalReadTillNow = (totalReadTillNow.substr(s+4, totalReadTillNow.size()-1)).c_str();
//                headerEnded = false;
//                bzero(buffer, SERVER_BUFFER_SIZE);
//                continue;
//            } else {
//                map<string, string> headersMap = get_headers_map(header);
//                int content_length = atoi(headersMap["Content-Length"].c_str());
//                string bodyAndNext = (totalReadTillNow.substr(s+4, totalReadTillNow.size()-1));
//                if (bodyAndNext.length() <= content_length) { // still need reading for request
//                    handle_post_request(req, client_fd, bodyAndNext.c_str(), header);
//                    header = "";
//                    totalReadTillNow = "";
//                    headerEnded = false;
//                } else {
//                    handle_post_request(req, client_fd, bodyAndNext.substr(0, content_length - 1).c_str(), header);
//                    header = "";
//                    totalReadTillNow = bodyAndNext.substr(content_length - 1, bodyAndNext.length() - 1);
//                    headerEnded = false;
//                }
//            }
//        } else { // header not ended
//            totalReadTillNow.append(buffer);
//        }
//        bzero(buffer, SERVER_BUFFER_SIZE);
//        n = read(client_fd, buffer, SERVER_BUFFER_SIZE - 1);
//    }
//
//    printf("%s\n", buffer);
//}

void handle_post_request(server_request request, int client_fd, const char* body, string header) {
    const char *response = "HTTP/1.1 200 OK\r\n\r\n";
    ssize_t n = write(client_fd, response, strlen(response));
    if (n < 0) {
        cout << "ERROR writing to socket\n";
        exit(1);
    }
    map<string, string> headersMap = get_headers_map(header);
    string file_name = get_file_name(request, headersMap);
    int content_length = atoi(headersMap["Content-Length"].c_str());
    content_length -= strlen(body);
    save_content(file_name, body, strlen(body), "w+");
    while (content_length > 0) {
        char buffer[SERVER_BUFFER_SIZE];
        bzero(buffer, SERVER_BUFFER_SIZE);
        n = read(client_fd, buffer, SERVER_BUFFER_SIZE - 1);
        if (n < 0) {
            cout << "ERROR reading from socket\n";
            return;
        }
        save_content(file_name, buffer, n, "a");
        content_length -= strlen(buffer);
    }
}

void save_content(string file_name, const char *content, const ssize_t len, string mode) {
    FILE *file_to_save = fopen(file_name.c_str(), mode.c_str());
    fwrite((void*) content, sizeof(char), sizeof(char) * ((int) len), file_to_save);
    fclose(file_to_save);
}

string get_file_name(const server_request &request, map<string,string> headersMap) {
    vector<string> tokens = split(request.file_name, '/');
    string name = tokens[tokens.size()-1];
    unsigned long s = name.find('.');
    if (s == string::npos) { // didn't find extension in the file name
        initialize_file_extensions_if_needed();
        string content_type = headersMap["Content-Type"];
        content_type = content_type.substr(0, content_type.size() - 1);
        name += "." + CONTENT_TO_FILE_EXTENSIONS[content_type];
    }
    return name;
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

void handle_get_request(server_request request, int client_fd) {
    string file_name = request.file_name.substr(1);
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
        return;
    } else {
        inFile.seekg (0, ios::end);
        long len = inFile.tellg();
        inFile.seekg (0, ios::beg);
        char* body = new char[len];
        inFile.read (body, len);
        inFile.close();

        vector<string> tokens = split(file_name, '.');
        if (tokens.size() == 1) tokens.emplace_back("");
        initialize_file_extensions_if_needed();
        string content_type = FILE_EXTENSIONS[tokens[1]];

        string headers = "HTTP/1.1 200 OK\r\nContent-Type: " + content_type
                          + "\r\nContent-Length: " + to_string(len) + "\r\n\r\n";
        ssize_t n = write(client_fd, headers.c_str(), strlen(headers.c_str()));
        write(client_fd, body, (int) len);
        if (n < 0) {
            cout << "ERROR writing to socket\n";
            exit(1);
        }
    }
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
