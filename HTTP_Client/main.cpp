#include <iostream>
#include <fstream>
#include "input_reader.h"
#include "sender.h"
#include "sockets_manager.h"

using namespace std;

void process_request(vector<request> requests);

int main() {
    cout << "Enter Input File Path!" << endl;
    string file_path;
//    cin >> file_path;
    file_path = "input.txt";
    vector<vector<request>> requests = read_requests_from_file(file_path);
    for (int i = 0; i < requests.size(); i++) {
        process_request(requests[i]);
    }
    return 0;
}

void process_request(vector<request> requests) {
    int sock_fd;
    int start_index = 0;
    int state = 0;
    while (state != -1) {
        sock_fd = get_socket_fd(requests[0]);
        // if state = -1 this means that all requests are successful
        state = send_request(sock_fd, requests, start_index);
//        cout << state << endl;
        start_index = state;
    }
}