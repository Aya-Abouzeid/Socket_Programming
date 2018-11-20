//
// Created by programajor on 11/16/18.
//

#ifndef SOCKET_PROGRAMMING_REQUEST_HANDLER_H
#define SOCKET_PROGRAMMING_REQUEST_HANDLER_H

#include <mutex>
#include <map>

void handle_request(int client_fd, std::mutex &mtx, std::map<int, std::chrono::system_clock::time_point> &open_sockets);

#endif //SOCKET_PROGRAMMING_REQUEST_HANDLER_H
