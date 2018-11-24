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
#include "server_constants.h"
#include "server_info.h"
#include "socket_manager.h"
#include "request_handler.h"
#include "timout_manager.h"
#include <unistd.h>
#include <cstring>
#include <thread>
#include <mutex>          // std::mutex
#include <condition_variable>

using namespace std;

int main(int argc, char* argv[]) {
    int newsockfd;
    u_short portno;
    socklen_t clilen;
    struct sockaddr_in cli_addr;
    mutex mtx;           // mutex for critical section

    map<int, chrono::system_clock::time_point> open_sockets;
    if (argc < 2) {
        fprintf(stderr,"ERROR, no port provided\n");
        exit(1);
    }
    portno = atoi(argv[1]);
    struct server server_info;
    server_info.IPaddress = INADDR_ANY;
    server_info.port_number = portno;
    int sockfd = get_socket_fd(server_info);

    if (listen(sockfd, 10000) < 0) {
        perror("listen failed");
    }

    cout << "Server running on port " << portno << " , waiting for new requests .......\n";
    while (true) {
        while (open_sockets.size() >= MAX_SIMULTANEOUS_CONNECTIONS);

        clilen = sizeof(cli_addr);
        if ((newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen)) < 0) {
            perror("accepting failed");
            break;
        }

        if (newsockfd < 0) {
            cout << "ERROR on accept\n";
            exit(1);
        }

        auto current_time = chrono::system_clock::now();

        thread handle_req(handle_request, newsockfd, ref(mtx), ref(open_sockets));

        // add current socket to open sockets map
        mtx.lock();
        open_sockets[newsockfd] = current_time;
        cout << "=================-------------Currently " << open_sockets.size() << " users connected-------------=================\n";
        mtx.unlock();

        handle_req.detach();
    }

    close(sockfd);
    return 0;
}