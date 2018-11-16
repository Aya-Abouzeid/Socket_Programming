/**
 * CS431 : Networks.
 * Assignment 1 : HTTP Client
 * @author Marc Magdi
 * Tuesday 30 October 2018
 */
#ifndef HTTP_CLIENT_CONSTANTS_H
#define HTTP_CLIENT_CONSTANTS_H

#include <string>
#include <map>


#define GET 0
#define POST 1

#define STATUS_CODE "Status-Code"
#define CONTENT_BODY "Content-Body"

const int BUFFER_SIZE = 512;
const std::string HEADER_END = "\r\n\r\n";
const std::map<std::string, std::string> EXTENSIONS = {{"image/jpeg", "jpg"}, {"image/png", "png"}, {"text/html", "html"}, {"text/plain", "txt"}, {"text/plain", ""}};

#endif //HTTP_CLIENT_CONSTANTS_H
