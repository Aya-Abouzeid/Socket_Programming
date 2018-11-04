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
#include <fstream>
#include <iostream>
#include "constants.h"
#include "request_parser.h"

string REQUEST_TYPES[2] = { "GET", "POST" };

map<string, string> get_headers(const string headers);
string get_header(request req);
string get_last_header(request req);

void send_request(int sock_fd, vector<request> requests_info) {
    ssize_t n = 0;
    char buffer[BUFFER_SIZE];
    for (int i = 0; i < requests_info.size(); i++) {
        auto req = requests_info[i];
        string header;

        // tell the server to close the connection if this is the last connection in the vector
        if (i == requests_info.size() - 1) {
            header = get_last_header(req);
        } else {
            header = get_header(req);
        }

        n = write(sock_fd, header.c_str(), strlen(header.c_str()));
        bzero(buffer, BUFFER_SIZE);
        bool headersEnded = false;
        string headers;
        FILE *file_to_save;
        map<string, string> headersMap;
        unsigned long content_length = 0;

        while (n > 0 && (!headersEnded || content_length != atol(headersMap.find("Content-Length").operator*().second.c_str()))) {
            n = read(sock_fd, buffer, BUFFER_SIZE - 1);
            string temp_received_data = headers + buffer;
            unsigned long s = temp_received_data.find(HEADER_END);

            if (headersEnded) { // appending body data to file
                fwrite((void *) buffer, sizeof(char), sizeof(char) * n, file_to_save);
                content_length += n;
            } else if (s != string::npos) { // finish reading header data
                headersEnded = true;
                string rest_of_header = temp_received_data.substr(0, s);
                headers.append(rest_of_header);
                headersMap = get_headers(headers);
                string body = temp_received_data.substr(s+4, temp_received_data.size()-1);
                char * temp_body = new char [body.length()+1];
                strcpy (temp_body, body.c_str());

                file_to_save = fopen("image.txt",  "w+");
                content_length = strlen(temp_body);
                fwrite((void*) temp_body, sizeof(char), sizeof(temp_body), file_to_save);
            } else { // header not ended
                headers.append(buffer);
            }

            bzero(buffer, BUFFER_SIZE);
        }

        if (n < 0) {
            perror("error getting data from server");
        }

        fclose(file_to_save);
    }
    close(sock_fd);
}

map<string, string> get_headers(const string headers) {
    stringstream ss(headers);
    string line;
    map<string, string> ret;
    while (getline(ss, line)) {
        unsigned long s = line.find(": ");
        if (s == string::npos) { // (:) not found, get http response code
            ret[STATUS_CODE] = split(line, ' ')[1];
        } else {
            ret[line.substr(0, s)] = line.substr(s+2, line.size()-1);
        }
    }
    return ret;
}

string get_last_header(request req) {
    return REQUEST_TYPES[req.request_type] + " " + req.file_name
           + " HTTP/1.0\r\nHost: " + req.host_name + "\r\nConnection: close\r\n\r\n";
}

string get_header(request req) {
    return REQUEST_TYPES[req.request_type] + " " + req.file_name
           + " HTTP/1.1\r\nHost: " + req.host_name + "\r\n\r\n";
}

string get_file_name(request req) {
    vector<string> tokens = split(req.file_name, '/');
    string name = tokens[tokens.size()-1];

    return tokens[tokens.size()-1];
}

string get_file_extension() {

}