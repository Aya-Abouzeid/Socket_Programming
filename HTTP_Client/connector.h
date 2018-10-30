/**
 * CS431 : Networks.
 * Assignment 1 : HTTP Client
 * @author Marc Magdi
 * Tuesday 30 October 2018
 */

#ifndef HTTP_CLIENT_CONNECTOR_H
#define HTTP_CLIENT_CONNECTOR_H

/**.
 * Use the socket file descriptor to send the request to the server.
 * @param sock_fd socket file descriptor connected to the server
 * @param request_info the request data to send to server
 */
void send_request(int sock_fd, struct request request_info);

#endif //HTTP_CLIENT_CONNECTOR_H
