#pragma once

#include <unordered_map>
#include <string>

class HttpResponse {
    private:
        // useful during serialization and deserialization
        char * serialized_response_;
        int serialized_response_length_;

        int _http_status_code;
        std::string _http_status_message;
        std::string _http_version;
        std::unordered_map<std::string, std::string> _http_response_headers;
        // std::string _http_response_body;
        char * _http_response_body; // can be variable sized
        int _http_response_body_length;

        int extractResponseParams(const std::string &);
        int extractResponseHeaders(const std::string &);
        
    public:
        HttpResponse();
        
        HttpResponse* setHttpStatusCode(int);
        HttpResponse* setHttpStatusMessage(std::string);
        HttpResponse* setHttpVersion(std::string);
        HttpResponse* addResponseHeaders(const std::string &, const std::string &);
        HttpResponse* setHttpResponseBody(const char *, int);

        const char * getHttpResponseBody();
        int getHttpResponseBodyLength();

        const char * getSerializedResponse();
        int getSerializedResponseLength();

        void setSerializedResponse(const char *, int);

        void serialize();
        int deserialize();

        void displayResponse();
        std::string getHeaderValue(const std::string &);
};