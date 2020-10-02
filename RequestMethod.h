#pragma once

#include <string>
#include <set>

namespace RequestMethod {
    enum Enum {
        GET,
        HEAD,
        POST,
        PUT,
        DELETE,
        CONNECT,
        OPTIONS,
        TRACE,
        PATCH
    };
}


class RequestMethod : map<RequestMethod::Enum, std::string> {
    RequestMethod{
        this->emplace(GET, "GET");
        this->emplace(HEAD, "HEAD");
        this->emplace(POST, "POST");
        this->emplace(PUT, "PUT");
        this->emplace(DELETE, "DELETE");
        this->emplace(CONNECT, "CONNECT");
        this->emplace(OPTIONS, "OPTIONS");
        this->emplace(TRACE, "TRACE");
        this->emplace(PATCH, "PATCH");
    }

public:
    std::string get(RequestMethod::Enum method) {
        return this->find(method);
    }



};