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
#include <HTTP_Client/constants.h>
#include <unistd.h>

using namespace std;


int main(int argc,char* argv[]) {
    int sockfd, newsockfd, port_number;
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
    newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr,  &clilen);
    if (newsockfd < 0) {
        cout << "ERROR on accept\n";
        exit(1);
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
