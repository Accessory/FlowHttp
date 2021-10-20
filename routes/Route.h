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
    Route(std::string path = ".*",
          std::string method = "GET"
                  ) : Method(std::move(method)), Path(std::move(path))
//        ,Method_String(method), Path_String(path)
    {}

    std::regex Method;
    std::regex Path;
//    std::string Method_String;
//    std::string Path_String;

    virtual bool Run(Request &request, Response &response, Socket &socket) {
        return defaultFunction(request, response, socket);
    }

    static bool
    defaultFunction(Request &request, Response &response, Socket &socket) { return false; }
};
