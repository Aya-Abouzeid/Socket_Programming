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

vector<request> getRequestsVector(ifstream &inFile);

vector<request> read_requests_from_file(string file_path) {
    ifstream inFile;
    inFile.open(file_path);
    if (!inFile) {
        return vector<request>();
    }
    return getRequestsVector(inFile);
}

vector<request> getRequestsVector(ifstream &inFile) {
    if (inFile.is_open()) {
        vector<request> ret;
        string line;
        while (getline(inFile, line)) {
            request req = parse_request(line);
            ret.push_back(req);
        }
        inFile.close();
        return ret;
    }
    return vector<request>();
}