/**
 * CS431 : Networks.
 * Assignment 1 : HTTP Server
 * @author Aya Abouzeid
 * Saturday 3 November 2018
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "socket_manager.h"

int get_socket_fd(struct server server_info) {

    int socket_fd;

    /*
         A sockaddr_in is a structure containing an internet address.
        This structure is defined in netinet/in.h.
    /**/
    struct sockaddr_in serv_addr;

    /*
        AF_INET -> Protocol Family of Socket Used
        S0CK_STREAM -> Socket Type
        0 -> default protocol should be used
    /**/
    socket_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if(socket_fd < 0) {
        perror("ERROR opening server socket");
        exit(EXIT_FAILURE);
    }

    // clear address structure
    memset(&serv_addr, 0, sizeof(serv_addr));

    // set address domain of the socket
    serv_addr.sin_family = AF_INET;

    serv_addr.sin_addr.s_addr = server_info.IPaddress;

    u_short port_number = server_info.port_number;

    serv_addr.sin_port = htons(port_number);

    /*
        binds a socket to address
         It takes three arguments,
         the socket file descriptor,
         the address to which is bound,
         and the size of the address to which it is bound
        0 -> default protocol should be used
    /**/
    if (bind(socket_fd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
        perror("ERROR on binding");

    return socket_fd;
}