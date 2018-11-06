/**
 * CS431 : Networks.
 * Assignment 1 : HTTP Server
 * @author Aya Abouzeid
 * Monday 5 November 2018
 */

#ifndef HTTP_Server_REQUEST_H
#define HTTP_Server_REQUEST_H

#include <string>

using namespace std;

/**.
 * a request struct describing any request initiated by the client.
 */
struct request {
    int client_fd;
    int request_type;
    string file_name;
    string body;
    string HTTPVer;
};

#endif //HTTP_Server_REQUEST_H
