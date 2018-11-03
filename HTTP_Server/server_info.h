/**
 * CS431 : Networks.
 * Assignment 1 : HTTP Server
 * @author Aya Abouzeid
 * Saturday 3 November 2018
 */

#ifndef SOCKET_PROGRAMMING_SERVER_INFO_H
#define SOCKET_PROGRAMMING_SERVER_INFO_H

#include <string>

using namespace std;

/**.
 * a request struct describing any request initiated by the client.
 */
struct server {
    u_long IPaddress;
    u_short port_number;
};

#endif //SOCKET_PROGRAMMING_SERVER_INFO_H
