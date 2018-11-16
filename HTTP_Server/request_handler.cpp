//
// Created by programajor on 11/16/18.
//

#include <strings.h>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <iostream>
#include <HTTP_Client/request.h>
#include <fstream>
#include "request_handler.h"
#include "server_constants.h"
#include "request_parser.h"

using namespace std;

void handle_get_request(server_request request, int client_fd);

int get_content_length(string basic_string);

string get_content_body(string file_name);

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
            struct server_request req = extract_request_params_from_header(header);
            if (req.request_type == GET) {
                handle_get_request(req, client_fd);
                return;
            } else {
                string body = totalReadTillNow.substr(s+4, totalReadTillNow.size()-1);
            }
        } else { // header not ended
            header.append(buffer);
        }
        bzero(buffer, SERVER_BUFFER_SIZE);
        n = read(client_fd, buffer, SERVER_BUFFER_SIZE - 1);
    }

    printf("%s\n", buffer);

    char* res = "HTTP/1.1 200 OK\r\nContent-Type: text\\html\r\nContent-Length: 25\r\nAccept-Ranges: bytes\r\nX-Cloud-Trace-Context: c19d80bb5bb1a6c5dafbfb23e4384b47\r\nDate: Fri, 16 Nov 2018 10:16:48 GMT\r\nServer: Google Frontend\r\nContent-Length: 4376\r\n\r\ntestinh asdafsdfasdf";
    n = write(client_fd, res, strlen(res));

    if (n < 0) {
        cout << "ERROR writing to socket\n";
        exit(1);
    }
}

void handle_get_request(server_request request, int client_fd) {
    ifstream inFile;
    inFile.open(request.file_name);
    if (!inFile) {
        const char *response = "HTTP/1.1 404 Not Found\r\n";
        ssize_t n = write(client_fd, response, strlen(response));
        if (n < 0) {
            cout << "ERROR writing to socket\n";
            exit(1);
        }
        return;
    } else {
        vector<string> tokens = split(request.file_name, '.');
        if (tokens.size() == 1) tokens.emplace_back("");
        string content_type = FILE_EXTENSIONS[tokens[1]];
        int content_length = get_content_length(request.file_name);
        string content_body = get_content_body(request.file_name);
    }
}

string get_content_body(string file_name) {
    return std::__cxx11::string();
}

int get_content_length(string file_name) {
    ifstream is;
    is.open (file_name.c_str(), ios::binary );
    is.seekg (0, ios::end);
    return is.tellg();
}
