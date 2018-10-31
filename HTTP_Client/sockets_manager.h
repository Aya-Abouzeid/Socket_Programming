/**
 * CS431 : Networks.
 * Assignment 1 : HTTP Client
 * @author Marc Magdi
 * Wednesday 31 October 2018
 */

#ifndef HTTP_CLIENT_SOCKETS_MANAGER_H
#define HTTP_CLIENT_SOCKETS_MANAGER_H


/**.
 * get socket file descriptor if already opened or open a new one if not.
 * @param request_info contains the server and the port to connect to
 * @return socket_fd of the created connection
 */
int get_socket_fd(struct request request_info);

#endif //HTTP_CLIENT_SOCKETS_MANAGER_H
