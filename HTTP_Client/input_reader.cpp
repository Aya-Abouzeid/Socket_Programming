/**
 * CS431 : Networks.
 * Assignment 1 : HTTP Client
 * @author Bishoy Nader Gendy
 * Wednesday 31 October 2018
 */

#include "input_reader.h"
#include "request_parser.h"
#include <fstream>
#include <iostream>

using namespace std;

vector<vector<request>> get_requests_vector(ifstream &inFile);

string get_key(const request &request);

vector<vector<request>> read_requests_from_file(string file_path) {
    ifstream inFile;
    inFile.open(file_path);
    if (!inFile) {
        return vector<vector<request>>();
    }
    return get_requests_vector(inFile);
}

vector<vector<request>> get_requests_vector(ifstream &inFile) {
    string key = "";
    vector<vector<request>> ret;
    if (inFile.is_open()) {
        string line;
        while (getline(inFile, line)) {
            request req = parse_request(line);
            string requestKey = get_key(req);
            if (requestKey != key) {
                vector<request> vec;
                ret.push_back(vec);
                key = requestKey;
            }
            ret[ret.size() - 1].push_back(req);
        }
        inFile.close();
        return ret;
    }
    return vector<vector<request>>();
}

string get_key(const request &req) {
    return req.host_name + "#" + to_string(req.port_number);
}