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

/**.
 * Map the header arguments to a map key and value, ex: [Content-Length] = 123
 * @param header the header as a string
 * @return the created map of keys and values
 */
map<string, string> get_headers(const string header);

/**.
 *
 * @param last_request_for_server identify if this is the last request to server,
 * if so it will add a connection close to the header to tell the server to close the connection.
 * @param req the request to create the heade for.
 * @return the header as a string to send to server in the request
 */
string get_header(bool last_request_for_server, request req);

string get_file_name(request req, map<string, string> headersMap);

void process_get_request(int sock_fd, request req, bool last_request_for_server);
void process_post_request(int sock_fd, request req, bool last_request_for_server);

void send_request(int sock_fd, vector<request> requests_info) {
    ssize_t n = 0;
    char buffer[BUFFER_SIZE];
    for (int i = 0; i < requests_info.size(); i++) {
        auto req = requests_info[i];
        if (req.request_type == GET)
            process_get_request(sock_fd, req, i == requests_info.size()-1);
        else if (req.request_type == POST)
            process_post_request(sock_fd, req, i == requests_info.size()-1);

//        string req_header = get_header(i, requests_info.size(), req);
//
//        if (req.request_type == GET) process_get_request();
//        else if (req.request_type == POST);
//        string req_header = get_header(i, requests_info.size(), req);
//        FILE *file;
//
//        n = send(sock_fd, req_header.c_str(), strlen(req_header.c_str()), 0);
//        bzero(buffer, BUFFER_SIZE);
//        bool headersEnded = false;
//        string headers;
//        FILE *file_to_save = fopen("image.jpg", "r");
//        map<string, string> headersMap;
//        unsigned long content_length = 0;
//
//        // check no error and didn't reached content length size
//        while (n > 0 && (!headersEnded || content_length != atol(headersMap.find("Content-Length").operator*().second.c_str()))) {
//            send(sock_fd, "Hello World", 11, 0);
//            n = read(sock_fd, buffer, BUFFER_SIZE - 1);
//            string temp_received_data = headers + buffer;
//            unsigned long s = temp_received_data.find(HEADER_END);
//
//            if (headersEnded) { // appending body data to file
//                fwrite((void *) buffer, sizeof(char), sizeof(char) * n, file_to_save);
//                content_length += n;
//            } else if (s != string::npos) { // finish reading header data
//                headersEnded = true;
//                string rest_of_header = temp_received_data.substr(0, s);
//                headers.append(rest_of_header);
//                headersMap = get_headers(headers);
//                string body = temp_received_data.substr(s+4, temp_received_data.size()-1);
//                char * temp_body = new char [body.length()+1];
//                strcpy (temp_body, body.c_str());
//
//                file_to_save = fopen("image.txt",  "w+");
//                content_length = strlen(temp_body);
//                fwrite((void*) temp_body, sizeof(char), sizeof(temp_body), file_to_save);
//            } else { // header not ended
//                headers.append(buffer);
//            }
//
//            bzero(buffer, BUFFER_SIZE);
//        }
//
//        if (n < 0) {
//            perror("error getting data from server");
//        }
//
//        fclose(file_to_save);
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

string get_header(bool last_request_for_server, request req) {
    string header;
    if (last_request_for_server) {
        header =  REQUEST_TYPES[req.request_type] + " " + req.file_name
                    + " HTTP/1.1\r\nHost: " + req.host_name + "\r\nConnection: close\r\n\r\n";
    } else {
        header = REQUEST_TYPES[req.request_type] + " " + req.file_name
                    + " HTTP/1.1\r\nHost: " + req.host_name + "\r\n\r\n";
    }

    return header;
}

void process_get_request(int sock_fd, request req, bool last_request_for_server) {
    string req_header = get_header(last_request_for_server, req);
    char buffer[BUFFER_SIZE];
    ssize_t n = write(sock_fd, req_header.c_str(), strlen(req_header.c_str()));
    bzero(buffer, BUFFER_SIZE);
    bool headersEnded = false;
    string header;
    FILE *file_to_save;
    map<string, string> headersMap;
    unsigned long content_length = 0;

    // check no error and didn't reached content length size
    while (n > 0 && (!headersEnded || content_length != atol(headersMap.find("Content-Length").operator*().second.c_str()))) {
        n = read(sock_fd, buffer, BUFFER_SIZE - 1);
        string temp_received_data = header + buffer;
        unsigned long s = temp_received_data.find(HEADER_END);

        if (headersEnded) { // appending body data to file
            fwrite((void *) buffer, sizeof(char), sizeof(char) * n, file_to_save);
            content_length += n;
        } else if (s != string::npos) { // finish reading header data
            headersEnded = true;
            string rest_of_header = temp_received_data.substr(0, s);
            header.append(rest_of_header);
            headersMap = get_headers(header);
            file_to_save = fopen(get_file_name(req, headersMap).c_str(),  "w+");
            content_length = n - s - 4;
            fwrite((void*) &buffer[s+4], sizeof(char), sizeof(char) * content_length, file_to_save);
        } else { // header not ended
            header.append(buffer);
        }

        bzero(buffer, BUFFER_SIZE);
    }

    if (n < 0) {
        perror("error getting data from server");
    }

    fclose(file_to_save);
}

void process_post_request(int sock_fd, request req, bool last_request_for_server) {

}

string get_file_extension(string const content_type) {
    vector<string> tokens = split(content_type, ';');

    string extension;
    if (tokens[0] == "image/jpeg") extension = "jpg";
    else if (tokens[0] == "image/png") extension = "png";
    else if (tokens[0] == "text/html") extension = "html";
    else if (tokens[0] == "text/plain") extension = "txt";

    return extension;
}

string get_file_name(request req, map<string, string> headersMap) {
    vector<string> tokens = split(req.file_name, '/');
    string name = tokens[tokens.size()-1];

    unsigned long s = name.find('.');
    if (s == string::npos) { // didn't find extension in the file name
        name += "." + get_file_extension(headersMap.find("Content-Type").operator*().second.c_str());
    }

    return name;
}