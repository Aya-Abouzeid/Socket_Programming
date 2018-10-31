//
// Created by marc on 10/30/18.
//

#ifndef HTTP_CLIENT_REQUEST_H
#define HTTP_CLIENT_REQUEST_H

#include <zconf.h>
#include <string>


/**.
 * a request struct describing any request initiated by the client.
 */
struct request {
    int request_type;
    char* file_name;
    char* host_name;
    char* port_number;
};

#endif //HTTP_CLIENT_REQUEST_H
