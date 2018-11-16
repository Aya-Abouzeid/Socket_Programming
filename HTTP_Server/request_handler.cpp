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

string get_content_body(string file_name, int content_length);

void initialize_file_extensions_if_needed();

map<string, string> FILE_EXTENSIONS;

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
    string file_name = request.file_name.substr(1);
    inFile.open(file_name);
    if (!inFile) {
        const char *response = "HTTP/1.1 404 Not Found\r\n";
        ssize_t n = write(client_fd, response, strlen(response));
        if (n < 0) {
            cout << "ERROR writing to socket\n";
            exit(1);
        }
        return;
    } else {
        vector<string> tokens = split(file_name, '.');
        if (tokens.size() == 1) tokens.emplace_back("");
        initialize_file_extensions_if_needed();
        string content_type = FILE_EXTENSIONS[tokens[1]];
        string content_length = to_string(get_content_length(file_name));
        string content_body = get_content_body(file_name, std::atoi(content_length.c_str()));
        string response = "HTTP/1.1 200 OK\r\nContent-Type: " + content_type
                          + "\r\nContent-Length: " + content_length + "\r\n\r\n" + content_body;
        ssize_t n = 1   ;
        int sz = response.length();
//        while (n < sz) {
            n = write(client_fd, response.c_str(), strlen(response.c_str()));
            cout << n << endl << strlen(response.c_str());
//            sz -= n;
//        }
        if (n < 0) {
            cout << "ERROR writing to socket\n";
            exit(1);
        }
    }
}

string get_content_body(string file_name, int content_length) {
    ifstream is;
    is.open (file_name, ios::binary | ios::in);
    is.seekg (0, is.beg);
    char* body = new char [content_length + 1];
    is.read (body, content_length);
    body[content_length - 1] = '\n';
    is.close();
    cout << body << endl;
    return body;
}

int get_content_length(string file_name) {
    ifstream is;
    is.open (file_name, ios::binary);
    is.seekg (0, ios::end);
    int len = is.tellg();
    is.close();
    return len;
}

void initialize_file_extensions_if_needed() {
    if (FILE_EXTENSIONS.size() == 0) {
        FILE_EXTENSIONS["jpg"] = "image/jpeg";
        FILE_EXTENSIONS["png"] = "image/png";
        FILE_EXTENSIONS["html"] = "text/html";
        FILE_EXTENSIONS["txt"] = "text/plain";
        FILE_EXTENSIONS[""] = "text/plain";
    }
}