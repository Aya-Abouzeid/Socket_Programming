/**
 * CS431 : Networks.
 * Assignment 1 : HTTP Client
 * @author Marc Magdi
 * Thursday 1 November 2018
 */

#include "sender.h"
#include "request.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <map>
#include <sstream>
#include "constants.h"
#include "request_parser.h"

string REQUEST_TYPES[2] = { "GET", "POST" };

const int BUFFER_SIZE = 512;

map<string, string> get_headers(const string headers);
string get_header(request req);

void send_request(int sock_fd, vector<request> requests_info) {
    ssize_t n = 0;
    char buffer[BUFFER_SIZE];
    for (auto req : requests_info) {
        string header = get_header(req);
        n = write(sock_fd, header.c_str(), strlen(header.c_str()));
        printf("%zi\n", n);
        bzero(buffer, BUFFER_SIZE);
        bool headersEnded = false;
        string headers;
        string body;
        while (n > 0) {
            n = read(sock_fd, buffer, BUFFER_SIZE);
            if (n == 0) break;
//            printf("%zi\n", n);
//            printf("%s\n", buffer);
            if (!headersEnded) {
                stringstream ss(buffer);
                string line;
                int headerlineNumber = 1;
                int bodyLineNumber = 1;
                while (getline(ss, line)) {
                    if (line == "\r") {
                        headersEnded = true;
                    } else if (headersEnded) {
                        if (bodyLineNumber == 1) {
                            body += line;
                        } else {
                            body += '\n' + line;
                        }
                        bodyLineNumber++;
                    } else {
                        if (headerlineNumber == 1) {
                            headers += line;
                        } else {
                            headers += '\n' + line;
                        }
                        headerlineNumber++;
                    }
                }
            } else {
                body += buffer;
            }
            bzero(buffer, BUFFER_SIZE);
        }
        map<string, string> headersMap = get_headers(headers);
        headersMap.size();
    }
    close(sock_fd);
}

map<string, string> get_headers(const string headers) {
    stringstream ss(headers);
    string line;
    map<string, string> ret;
    while (getline(ss, line)) {
        vector<string> tokens = split(line, ':');
        if (tokens.size() == 1) {
            ret[STATUS_CODE] = split(tokens[0], ' ')[1];
        } else {
            ret[tokens[0]] = tokens[1];
        }
    }
    return ret;
}

string get_header(request req) {
    return REQUEST_TYPES[req.request_type] + " " + req.file_name
           + " HTTP/1.0\r\nHost: " + req.host_name + "\r\n\r\n";
}