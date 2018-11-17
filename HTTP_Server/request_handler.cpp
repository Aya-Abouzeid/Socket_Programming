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
    bzero(buffer, SERVER_BUFFER_SIZE);
    bool headerEnded = false;
    string header = "";
    string totalReadTillNow = "";
    ssize_t n = read(client_fd, buffer, SERVER_BUFFER_SIZE - 1);

    if (n < 0) {
        cout << "ERROR reading from socket\n";
        exit(1);
    }

    while (n > 0 && !headerEnded) {
        totalReadTillNow += buffer;
        unsigned long s = totalReadTillNow.find(REQUEST_HEADER_END);

        if (headerEnded) { // appending body data to file

        } else if (s != string::npos) { // finish reading header data
            headerEnded = true;
            header = totalReadTillNow.substr(0, s);
            cout << totalReadTillNow;
            struct server_request req = extract_request_params_from_header(header);
            if (req.request_type == GET) {
                handle_get_request(req, client_fd);
                return;
            } else {
                handle_post_request(req, client_fd, (totalReadTillNow.substr(s+4, totalReadTillNow.size()-1)).c_str(), header);
                return;
            }
        } else { // header not ended
            totalReadTillNow.append(buffer);
        }
        bzero(buffer, SERVER_BUFFER_SIZE);
        n = read(client_fd, buffer, SERVER_BUFFER_SIZE - 1);
    }

    printf("%s\n", buffer);
}

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
