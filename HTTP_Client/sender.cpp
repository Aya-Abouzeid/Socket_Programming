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
    char *header = "GET /t/4pa1k-1541106672 HTTP/1.1\nHost: www.ptsv2.com\n";
    n = write(sock_fd,header, strlen(header));
    printf("%zi\n", n);
    n = read(sock_fd,header,255);
    printf("%zi\n", n);
    printf("%s\n",header);
    close(sock_fd);
}