/**
 * CS431 : Networks.
 * Assignment 1 : HTTP Client
 * @author Marc Magdi
 * Tuesday 30 October 2018
 */

#ifndef HTTP_CLIENT_REQUEST_H
#define HTTP_CLIENT_REQUEST_H

#include <string>

using namespace std;

/**.
 * a request struct describing any request initiated by the client.
 */
struct request {
    int request_type;
    string file_name;
    string host_name;
    u_short port_number;
};

#endif //HTTP_CLIENT_REQUEST_H
