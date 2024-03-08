#include "request.h"
#include "../utils/utils.h"

#include <sstream>
#include <regex>
#include <iostream>

// constructors
HttpRequest::HttpRequest() {
}

// getters and setters
std::string HttpRequest::getHttpRequestMethod() const {
    return _http_request_param.method;
}

std::string HttpRequest::getHttpRequestUri() const {
    return _http_request_param.uri;
}

const char * HttpRequest::getHttpVersion() const {
    return _http_version;
}

void HttpRequest::setHttpRequestBody(const char * request_body, int length) {
    _http_request_body_length = length;

    // dynamically set the response body char pointer
    _http_request_body = new char[length + 1];
    memset(_http_request_body, 0, length + 1);
    strcpy(_http_request_body, request_body);
}

const char * HttpRequest::getHttpRequestBody() {
    return _http_request_body;
}

int HttpRequest::getHttpRequestBodyLength() {
    return _http_request_body_length;
}

const char * HttpRequest::getSerializedRequest() {
    return serialized_request_;
}

int HttpRequest::getSerializedRequestLength() {
    return serialized_request_length_;
}

void HttpRequest::setSerializedRequest(const char *request, int length) {
    serialized_request_ = new char[length + 1];
    memset(serialized_request_, 0, length + 1);
    strcpy(serialized_request_, request);
    serialized_request_length_ = length;
}

/*
Build a new Http Request
*/
HttpRequest * HttpRequest::build(std::string request_string) {
    HttpRequest *http_request = new HttpRequest;

    // set the request string
    http_request->setSerializedRequest(request_string.c_str(), request_string.size());

    // deserialize it
    if (http_request->deserialize() == -1) {
        return nullptr;
    }

    return http_request;
}

/*
    Deserialize a stream of character array into a Http Request object
    HTTP Format for requests
    Request Line
    Request Headers
    Request Body
*/
int HttpRequest::deserialize() {
    std::cout << " ---------- Inside request deserializer ---------- " << std::endl;
    
    // split this request std::string by \n
    std::stringstream ss(serialized_request_);
    std::string request_line;

    // extract request line parameters
    getline(ss, request_line, '\r');
    if (extractRequestParams(request_line) == -1) {
        perror("Failed to extract request line parameters");
        return -1;
    }
    
    if (ss.peek() == '\n') {
        ss.ignore();
    }

    // extract request line headers
    while (getline(ss, request_line, '\r')) {
        if (request_line.length() == 0) {
            break;
        }
        if (extractRequestHeaders(request_line) == -1) {
            perror("Failed to extract request header");
            return -1;
        }
        if (ss.peek() == '\n') {
            ss.ignore();
        }
    }
    if (ss.peek() == '\n') {
        ss.ignore();
    }

    std::cout << "Trying to check URI pattern" << std::endl;
    // check if the URI is absolute path, change to relative path with respect to host header
    int pos;
    if ((pos = _http_request_param.uri.find(_http_request_headers["Host"])) != std::string::npos) {
        // update the URI
        _http_request_param.uri = _http_request_param.uri.substr(pos + _http_request_headers["Host"].length());
        std::cout << "Updated check on URI pattern" << std::endl;
    }

    std::cout << "Trying to get content length from header" << std::endl;

    // check for the content length header and extract body of the response
    std::string content_length = getHeaderValue("Content-Length");
    if (content_length != "") {
        std::cout << "Found content length" << std::endl;
        _http_request_body_length = stoi(content_length);
        _http_request_body = new char[_http_request_body_length + 1];
        memset(_http_request_body, 0, _http_request_body_length + 1);
        std::cout << "Trying to get data from ss readsome" << std::endl;
        ss.readsome(_http_request_body, _http_request_body_length);
    }
    
    std::cout << " ----------- Finished request deserializer ----------- " << std::endl;
    return 0;
}

int HttpRequest::extractRequestParams(std::string request_line) {
    // regex pattern for HTTP Request line
    std::regex requestPattern("(GET|POST|PUT|DELETE|HEAD|OPTIONS|PATCH|TRACE)\\s(\\S+)\\sHTTP/(\\S+)");

    // Create a match object to store the matched components
    std::smatch requestMatch;

    if (regex_search(request_line, requestMatch, requestPattern)) {
        // set the necessary parameters here
        if (requestMatch.size() == 4) {
            _http_request_param.method = requestMatch[1].str();
            _http_request_param.uri = requestMatch[2].str();            
            strcpy(_http_version, requestMatch[3].str().c_str());
            return 0;
        }
    }
    return -1;
}

int HttpRequest::extractRequestHeaders(std::string request_line) {
    // assume it's headers (key: value)
    int delim_pos = -1;
    if ((delim_pos = findPosition(request_line, ":")) == -1) {
        return -1;
    }
    // extract parts of the std::string
    _http_request_headers[request_line.substr(0, delim_pos)] = request_line.substr(delim_pos + 2);

    return 0;
}

void HttpRequest::displayRequest() {
    std::ostringstream request;
    request << "Request : { Method: " << _http_request_param.method << ", URI: " << _http_request_param.uri << ", Version: " << _http_version << " }" << std::endl;
    std::cout << request.str();
}

std::string HttpRequest::getHeaderValue(const std::string &header_key) {
    for (const auto& header : _http_request_headers) {
        if (header_key == header.first) {
            return header.second;
        }
    }
    return "";
}