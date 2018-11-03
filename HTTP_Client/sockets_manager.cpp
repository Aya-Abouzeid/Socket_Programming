/**
 * CS431 : Networks.
 * Assignment 1 : HTTP Client
 * @author Aya Abouzeid
 * Wednesday 31 October 2018
 */

#include "sockets_manager.h"
#include "request.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <netdb.h>

int get_socket_fd(struct request request_info) {

    int socket_fd;

    /*
         A sockaddr_in is a structure containing an internet address.
        This structure is defined in netinet/in.h.
    /**/
    struct sockaddr_in serv_addr;

    /*
        structure is defined in the header file netdb.h
    /**/
    struct hostent *server;

    /*
        AF_INET -> Protocol Family of Socket Used
        S0CK_STREAM -> Socket Type
        0 -> default protocol should be used
    /**/
    socket_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(socket_fd < 0) {
        perror("ERROR opening client socket");
        exit(EXIT_FAILURE);
    }

    string host_name = request_info.host_name;

    u_short port_number = request_info.port_number;

        server = gethostbyname(host_name.c_str());

        if (server == NULL) {
            fprintf(stderr, "ERROR, no such host\n");
            exit(EXIT_FAILURE);
        }

        // clear address structure
        memset(&serv_addr, 0, sizeof(serv_addr));

        bcopy((char *) server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);

        serv_addr.sin_port = htons(port_number);

        // set address domain of the socket
        serv_addr.sin_family = AF_INET;

        if (connect(socket_fd, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) != 0)
            perror("ERROR connecting");



    return socket_fd;
}
