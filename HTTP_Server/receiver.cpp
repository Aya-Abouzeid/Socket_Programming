/**
 * CS431 : Networks.
 * Assignment 1 : HTTP Server
 * @author Aya Abouzeid
 * Saturday 3 November 2018
 */

#include "receiver.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

void receive(int socket_fd){

    // buffer for server to read into
    char buffer[10000];

    // number of chars read into buffer
    int char_read = 0;

    /*
           reference pointer to the address of the
           client on the other end of the connection
    /**/
    struct sockaddr_in client_addr;

    socklen_t client_len;

    /*
        listen on the socket for connections
        first argument is the socket file descriptor
        the second is the number of connections that can wait
     /**/
    if(listen(socket_fd, 10) < 0) {
        perror("listen failed");
    }

    while(true) {
        client_len = sizeof(client_addr);

        /*
            causes the process to block until a client
            connects to the server.
            returns new file descriptor
         /**/
        int new_socket_fd = accept(socket_fd,
                                 (struct sockaddr *) &client_addr, &client_len);

        if (new_socket_fd < 0)
            perror("ERROR on accept");

        pid_t PID = fork();

        if (PID == 0) {

            bzero(buffer,10000);

            while ((char_read = read(new_socket_fd, buffer, sizeof(buffer))) > 0) {


            }

            if (char_read < 0)
                perror("ERROR reading from socket");

            // exit process
            exit(0);
        } else {

            // close the new file descriptor
            close(new_socket_fd);

        }
    }

}