#pragma once

#include <unordered_map>
#include <string>

struct HttpRequestParam {
    std::string method;
    std::string uri;
};

struct HttpServerAddr {
    std::string domain_name;
    std::string port;
    std::string domain_ip;
};

class HttpRequest {
    private:
        // useful during serialization and deserialization
        char * serialized_request_;
        int serialized_request_length_;

        struct HttpRequestParam _http_request_param;
        char _http_version[3];
        std::unordered_map<std::string, std::string> _http_request_headers;
        char * _http_request_body; // can be variable sized
        int _http_request_body_length;

        // Private member functions to extract and construct the HttpRequest object
        int extractRequestParams(std::string);
        int extractRequestHeaders(std::string);
        int extractRequestBody(std::string);

    public:
        HttpRequest();
        
        static HttpRequest * build(std::string);

        std::string getHttpRequestMethod() const;
        std::string getHttpRequestUri() const;
        const char * getHttpVersion() const;

        void setHttpRequestBody(const char *, int);

        const char * getHttpRequestBody();
        int getHttpRequestBodyLength();

        const char * getSerializedRequest();
        int getSerializedRequestLength();
        void setSerializedRequest(const char *, int);

        int deserialize();

        void displayRequest();

        std::string getHeaderValue(const std::string &);
};