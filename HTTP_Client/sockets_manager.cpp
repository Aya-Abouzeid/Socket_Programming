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
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string>
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
        error("ERROR opening socket");
        exit(EXIT_FAILURE);
    }

        char* host_name = request_info.host_name;

        server = gethostbyname(host_name);

        if (server == NULL) {
            fprintf(stderr, "ERROR, no such host\n");
            exit(EXIT_FAILURE);
        }
        // clear address structure
        bzero((char *) &serv_addr, sizeof(serv_addr));

        // set address domain of the socket

        char *port_number = request_info.port_number;

        bcopy((char *) server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);

        serv_addr.sin_port = htons(atoi(port_number));

        serv_addr.sin_family = AF_INET;

        if (connect(sockfd, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) != 0)
            error("ERROR connecting");

    return socket_fd;
}
