/**
 * CS431 : Networks.
 * Assignment 1 : HTTP Client
 * @author Marc Magdi
 * Tuesday 30 October 2018
 */

#ifndef HTTP_CLIENT_CONNECTOR_H
#define HTTP_CLIENT_CONNECTOR_H

#include <vector>
#include "request.h"

/**.
 * Use the socket file descriptor to send the request to the server.
 * @param sock_fd socket file descriptor connected to the server
 * @param request_info the request data to send to server
 * @param start_index the index of the first request to start sending from it.
 * @return -1 in case of success else return index of first failed request
 */
int send_request(int sock_fd, vector<request> request_info, int start_index);

#endif //HTTP_CLIENT_CONNECTOR_H
