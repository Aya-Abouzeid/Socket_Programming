/**
 * CS431 : Networks.
 * Assignment 1 : HTTP Server
 * @author Marc Magdi
 * Tuesday 30 October 2018
 */
#ifndef HTTP_Server_CONSTANTS_H
#define HTTP_Server_CONSTANTS_H

#include <string>
#include <map>

#define GET 0
#define POST 1

#define STATUS_CODE "Status-Code"
#define CONTENT_BODY "Content-Body"

const std::string REQUEST_HEADER_END = "\r\n\r\n";
const int SERVER_BUFFER_SIZE = 1024;

const std::map<std::string, std::string> FILE_EXTENSIONS = {{"image/jpeg", "jpg"},
                                                       {"image/png", "png"},
                                                       {"text/html", "html"},
                                                       {"text/plain", "txt"},
                                                       {"text/plain", ""}};

#endif //HTTP_Server_CONSTANTS_H
