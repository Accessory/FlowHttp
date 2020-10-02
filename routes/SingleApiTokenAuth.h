#pragma once

#include "Route.h"
#include <FlowUtils/FlowLog.h>
#include <string>
#include <boost/asio.hpp>
#include "../FlowAsio.h"
#include <FlowUtils/base64.h>

class SingleApiTokenAuth : public Route {
public:
    SingleApiTokenAuth(const std::string token) : Route(".*", ".*") {
        _token = token;
    }

private:
    std::string _token;
    virtual bool Run(Request &request, Response &response, Socket &socket) {
        auto token = request.Header("X-Api-Token");
        if(_token != token){
            FlowAsio::sendUnauthorized(socket);
            return true;
        }
        return false;
    }
};