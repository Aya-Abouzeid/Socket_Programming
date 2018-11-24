/**
 * CS431 : Networks.
 * Assignment 1 : HTTP Server
 * @author Marc Magdi
 * Tuesday 30 October 2018
 */

#ifndef HTTP_Server_REQUEST_PARSER_H
#define HTTP_Server_REQUEST_PARSER_H

#include "request.h"
#include <vector>

using namespace std;

/**.
 *
 * @param request_line a request a string to parse and build a request object from
 * ex of line: GET file-name host-name (port-number)
 * @return object of request created with all fields initiated
 */

server_request extract_request_params_from_header(string header);

vector<string> split(const string &s, char delim);

#endif //HTTP_Server_REQUEST_PARSER_H
