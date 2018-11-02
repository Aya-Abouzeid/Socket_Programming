/**
 * CS431 : Networks.
 * Assignment 1 : HTTP Client
 * @author Marc Magdi
 * Thursday 1 November 2018
 */

#include "sender.h"
#include "request.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

void send_request(int sock_fd, vector<request> requests_info) {
    ssize_t n;
    char buffer[256];
    char *header = "GET /t/wb4aa-1541146219/post HTTP/1.1\r\nHost: ptsv2.com\r\n\r\n";
    n = write(sock_fd,header, strlen(header));
    printf("%zi\n", n);
    bzero(buffer,256);
    n = read(sock_fd,buffer,255);
    printf("%zi\n", n);
    printf("%s\n",buffer);
    close(sock_fd);
}