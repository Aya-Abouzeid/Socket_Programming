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
#include "constants.h"

string REQUEST_TYPES[2] = { "GET", "POST" };

void send_request(int sock_fd, vector<request> requests_info) {
    ssize_t n = 0;
    char buffer[512];
    for (auto req : requests_info) {
        string header = REQUEST_TYPES[req.request_type] + " " + req.file_name
                        + " HTTP/1.1\r\nHost: " + req.host_name + "\r\n\r\n";
        n = write(sock_fd, header.c_str(), strlen(header.c_str()));
        printf("%zi\n", n);
        bzero(buffer, 512);
        while (n != -1) {
            n = read(sock_fd, buffer, 255);
            printf("%zi\n", n);
            printf("%s\n", buffer);
            bzero(buffer, 256);
        }
    }
    close(sock_fd);
}