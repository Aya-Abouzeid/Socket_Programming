#include <iostream>
#include "input_reader.h"

using namespace std;

int main() {
    cout << "Enter Input File Path!" << endl;
    string file_path;
    cin >> file_path;
    vector<request> requests = read_requests_from_file(file_path);
    return 0;
}
