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
#include <algorithm>
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
string get_file_type(string const file_name);
char*  process_get_request(int sock_fd, request req, char* firstBufferPart);
char*  process_post_request(int sock_fd, request req, char* firstBufferPart);
void send_get_request(int sock_fd, request req, bool last_request_for_server);
void send_post_request(int sock_fd, request req, bool last_request_for_server);
char* append(const char *s, const char* c);

void process_data(request req, char* buffer, long remaining_content_length, int n, FILE *file_to_save) {
    int length = n < remaining_content_length ? n : remaining_content_length;

    if (req.request_type == POST) {
        string str_buffer = buffer;
        cout << str_buffer.substr(0, length);
    } else if (req.request_type == GET) {
        fwrite((void *) buffer, sizeof(char), sizeof(char) * length, file_to_save);
    }
}

void process_header(request req, char* buffer, long remaining_content_length, int n, FILE **file_to_save,
        int start, map<string, string> headersMap) {
    int length = n < remaining_content_length ? n : remaining_content_length;

    if (req.request_type == POST) {
        string str_buffer = buffer;
        cout << str_buffer.substr(0, start + 4 + length);
    } else if (req.request_type == GET) {
        *file_to_save = fopen(get_file_name(req, headersMap).c_str(), "w+");
        fwrite((void *) &buffer[start + 4], sizeof(char), sizeof(char) * length, *file_to_save);
    }
}

void send_request(int sock_fd, vector<request> requests_info) {
    // send all requests
    for (int i = 0; i < requests_info.size(); i++) {
        auto req = requests_info[i];
        if (req.request_type == GET)
            send_get_request(sock_fd, req, i == requests_info.size()-1);
        else if (req.request_type == POST)
            send_post_request(sock_fd, req, i == requests_info.size()-1);
    }


    // read all responses
    char current_buffer[BUFFER_SIZE];

    // have the previous buffer of the request
    char* buffer = "";
    int buffer_size = 0;
    ssize_t n = 1;
    bzero(current_buffer, BUFFER_SIZE);
    FILE *file_to_save = nullptr;

    for (int i = 0; i < requests_info.size(); i++) {
        auto req = requests_info[i];
        bool headersEnded = false;
        long remaining_content_length = 1;
        map<string, string> headersMap;
        bool firstLoop = true;

        // check no error and didn't reached content length size
        while (n > 0 && (!headersEnded || remaining_content_length > 0)) {
            bzero(current_buffer, BUFFER_SIZE);
            string temp_received_data;

            // check if there is a buffer already exist, use its data and don't get
            // data from the socket buffer.
            if (i != 0 && firstLoop && buffer_size != 0) {
                firstLoop = false;
                n = buffer_size;
                memcpy(current_buffer, buffer, buffer_size);
                buffer = "";
            } else {
                n = read(sock_fd, current_buffer, BUFFER_SIZE - 1);
                temp_received_data.append(buffer);
                if (n < 0) {
                    perror("error getting data from server"); exit(1);
                }
            }

            // contains the buffer for the request from start to current character.
            temp_received_data.append(current_buffer);
            unsigned long s = temp_received_data.find(HEADER_END);
            if (headersEnded) { // appending body data to file
                process_data(req, current_buffer, remaining_content_length, n, file_to_save);
                remaining_content_length -= n;
            } else if (s != string::npos) { // finish reading header data
                headersEnded = true;
                string rest_of_header = temp_received_data.substr(0, s);
                buffer = append(buffer, rest_of_header.c_str());
                headersMap = get_headers(buffer);
                remaining_content_length = atol(headersMap.find("Content-Length").operator*().second.c_str());
                int readed_body_length = n - s - 4 > remaining_content_length ? remaining_content_length : n - s - 4;
                remaining_content_length -= (n - s - 4);
                process_header(req, current_buffer, readed_body_length, n, &file_to_save, s, headersMap);
            } else { // header not ended
                buffer = append(buffer, current_buffer);
            }

        }

        // only close the file if GET
        if (req.request_type == GET)
            fclose(file_to_save);

        // pass current buffer data of next request to next request
        if (remaining_content_length < 0) {
            int new_request_buffer_size = remaining_content_length * -1;
            buffer_size = new_request_buffer_size;
            buffer = new char[new_request_buffer_size];
            memcpy(buffer, current_buffer + n - new_request_buffer_size, new_request_buffer_size);
        } else {
            buffer = "";
        }
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

string post_header(bool last_request_for_server, request req, long file_size, string content_type) {
    string header;
    if (last_request_for_server) {
        header = REQUEST_TYPES[req.request_type] + " " + req.file_name
                  + " HTTP/1.1\r\nHost: " + req.host_name + "\r\nContent-Type: " + content_type + "\r\nContent-Length: "
                  + to_string(file_size) + "\r\nConnection: close\r\n\r\n";
    } else {
        header = REQUEST_TYPES[req.request_type] + " " + req.file_name
                 + " HTTP/1.1\r\nHost: " + req.host_name + + "\r\nContent-Type: " + content_type + "\r\nContent-Length: "
                 + to_string(file_size) + "\r\n\r\n";
    }

    return header;
}

void send_get_request(int sock_fd, request req, bool last_request_for_server) {
    string req_header = get_header(last_request_for_server, req);
    ssize_t n = write(sock_fd, req_header.c_str(), strlen(req_header.c_str()));
    if (n < 0) {
        perror("error getting data from server");
    }
}

void send_post_request(int sock_fd, request req, bool last_request_for_server) {
    ifstream is;
    is.open (req.file_name, ios::binary);
//    is.open ("POST", ios::binary);
    // get length of file:
    is.seekg (0, ios::end);
    long file_size = is.tellg();
    is.seekg (0, ios::beg);

    char* send_buffer = new char [file_size];
    string xx = send_buffer;
    is.read (send_buffer, file_size);
    is.close();

    string content_type = get_file_type(req.file_name);
    string req_header = post_header(last_request_for_server, req, file_size, content_type);

    ssize_t n = write(sock_fd, req_header.c_str(), strlen(req_header.c_str()));

    if (n < 0) {
        perror("error sending data from server");
    }

    // send file data
    n = write(sock_fd, send_buffer, file_size);


    if (n < 0) {
        perror("error sending data from server");
    }

}

string get_file_type(string const file_name) {
    vector<string> tokens = split(file_name, '.');

    std::map<string, string>::const_iterator it;
    string extension = "text/plan";

    for (it = EXTENSIONS.begin(); it != EXTENSIONS.end(); ++it) {
        if (it->second == tokens[tokens.size()-1]) {
            extension = it->first;
            break;
        }
    }

    return extension;
}

string get_file_extension(string const content_type) {
    vector<string> tokens = split(content_type, ';');
    string extension = EXTENSIONS.find(tokens[0]).operator*().second;
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

char* append(const char *s, const char* c) {
    size_t lenS = strlen(s);
    size_t lenC = strlen(c);
    char buf[lenS+lenC];
    memcpy(buf, s, lenS);
    memcpy(buf + lenS, c, lenC);

    return strdup(buf);
}