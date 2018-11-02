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
    ssize_t n = 0;
    char buffer[512];
    char *header = "GET /t/mhg3x-1541148013/post HTTP/1.1\r\nHost: ptsv2.com\r\n\r\n";
    n = write(sock_fd,header, strlen(header));
    printf("%zi\n", n);
    bzero(buffer,512);
    while (n != -1) {
        n = read(sock_fd,buffer,255);
        printf("%zi\n", n);
        printf("%s\n",buffer);
        bzero(buffer,256);
    }
    close(sock_fd);
}