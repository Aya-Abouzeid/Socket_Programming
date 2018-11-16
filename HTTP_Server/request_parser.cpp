/**
 * CS431 : Networks.
 * Assignment 1 : HTTP Server
 * @author Bishoy Nader Gendy
 * Wednesday 31 October 2018
 */

#include <cstring>
#include <vector>
#include <sstream>
#include "request_parser.h"
#include "server_constants.h"

template<typename Out>
void split(const std::string &s, char delim, Out result) {
    stringstream ss(s);
    string item;
    while (std::getline(ss, item, delim)) {
        *(result++) = item;
    }
}

vector<string> split(const string &s, char delim) {
    vector<string> elems;
    split(s, delim, std::back_inserter(elems));
    return elems;
}

server_request parse_request(string request_line, int client_fd) {
    server_request req;
    vector<string> tokens = split(request_line, ' ');
    tokens[0] == "GET" ? req.request_type = GET : req.request_type= POST;
    req.file_name = tokens[1];
    req.HTTPVer = tokens[2];
    req.client_fd = client_fd;
    return req;
}

server_request extract_request_params_from_header(string header) {
    server_request req;
    vector<string> tokens = split(header, ' ');
    tokens[0] == "GET" ? req.request_type = GET : req.request_type = POST;
    req.file_name = tokens[1];
    return req;
}