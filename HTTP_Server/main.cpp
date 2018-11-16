/**
 * CS431 : Networks.
 * Assignment 1 : HTTP Server
 * @author Aya Abouzeid
 * Saturday 3 November 2018
 */

#include <iostream>
#include <fstream>
#include <netinet/in.h>
#include <strings.h>
#include <constants.h>
#include <unistd.h>

using namespace std;

void *connection_handler(void *client_socket);

int main(int argc,char* argv[]) {
    int sockfd, *newsockfd, port_number;
    struct sockaddr_in serv_addr, cli_addr;

    if (argc > 1) {
        port_number = atoi(argv[1]);;
    } else {
        cout << "ERROR, no port provided\n";
        exit(1);
    }

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        cout << "ERROR opening socket\n";
        exit(1);
    }
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(port_number);

    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        cout << "ERROR on binding\n";
        exit(1);
    }
    listen(sockfd, 5);
    socklen_t clilen = sizeof(cli_addr);
    while (newsockfd = accept(sockfd, (struct sockaddr*) &cli_addr, (socklen_t*) &clilen)) {
        cout << "Connection accepted";
        pthread_t thread;
        newsockfd = malloc(1);
        *newsockfd = cli_addr;
        if (pthread_create( &thread, NULL, connection_handler, (void*) newsockfd) < 0) {
            cout << "could not create thread";
            return 1;
        }
        cout << "Handler assigned";
    }

    char buffer[BUFFER_SIZE];
    bzero(buffer, BUFFER_SIZE);
    int n = read(newsockfd, buffer, BUFFER_SIZE - 1);
    if (n < 0) {
        cout << "ERROR reading from socket\n";
        exit(1);
    }
    printf("Here is the message: %s\n", buffer);

    n = write(newsockfd,"I got your message",18);
    if (n < 0) {
        cout << "ERROR writing to socket\n";
        exit(1);
    }
    close(newsockfd);
    close(sockfd);
    return 0;
}

void *connection_handler(void *client_socket) {

}