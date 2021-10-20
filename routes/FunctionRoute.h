#pragma once

#include <map>
#include <string>
#include <functional>
#include "../Request.h"
#include <boost/asio.hpp>
#include <regex>
#include <utility>
#include "../Socket.h"
#include "../Response.h"
#include "Route.h"


class FunctionRoute : public Route {
public:
    FunctionRoute(
            std::function<bool(Request &request, Response &response, Socket &socket)> toRun,
            const std::string &path = "/",
            const std::string &method = "GET"
    ) : toRun(std::move(toRun)), Route(path, method) {
    }

    std::function<bool(Request &request, Response &response, Socket &socket)> toRun;

    virtual bool Run(Request &request, Response &response, Socket &socket) {
        return toRun.operator()(request, response, socket);
    }
};
