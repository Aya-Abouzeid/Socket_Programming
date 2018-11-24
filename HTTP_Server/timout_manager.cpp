//
// Created by marc on 11/20/18.
//
#include "timout_manager.h"
#include <thread>
#include <mutex>          // std::mutex
#include "server_constants.h"
#include <netinet/in.h>
#include <iostream>

using namespace std;

/**.
 * Update the timeout for all opened sockets in the given map
 * @param mtx  mutex for synchronizations
 * @param open_sockets map of open sockets
 */
void update_timeout(std::mutex &mtx, std::map<int, std::chrono::system_clock::time_point> &open_sockets) {
    int timeout_sec = 3 * MAX_SIMULTANEOUS_CONNECTIONS / (open_sockets.size()+1);
    struct timeval timeout;
    timeout.tv_sec = timeout_sec+1;
    timeout.tv_usec = 0;

    for (map<int, std::chrono::system_clock::time_point>::iterator it = open_sockets.begin(); it != open_sockets.end(); it++ )
    {
        setsockopt(it->first, SOL_SOCKET, SO_RCVTIMEO, (const char *) &timeout, sizeof timeout);
    }
}