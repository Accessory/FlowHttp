#pragma once

#include <map>
#include <string>
#include <functional>
#include "../Request.h"
#include <boost/asio.hpp>
#include <regex>
#include "../Socket.h"
#include "../Response.h"


class Route {
public:
    Route(const std::string &path,
          const string &method = "GET"
    ) {
        this->Method = string(method);
        this->Path = string(path);
    }

    std::regex Method;
    std::regex Path;

    virtual bool Run(Request &request, Response &response, Socket &socket) {
        return defaultFunction(request, response, socket);
    }

    static bool
    defaultFunction(Request &request, Response &response, Socket &socket) { return false; }
};
