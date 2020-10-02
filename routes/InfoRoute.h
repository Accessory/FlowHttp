#pragma once

#include "Route.h"
#include <FlowUtils/FlowLog.h>
#include <string>
#include <boost/asio.hpp>
#include "../Socket.h"

class InfoRoute : public Route {
public:
    InfoRoute() : Route(".*", ".*") {}

    virtual bool Run(Request &request, Response &response, Socket &socket) {
        std::string ip;
        if(socket.IsSSL()){
            ip = socket.GetSSLSocket()->lowest_layer().remote_endpoint().address().to_string();
        } else {
            ip = socket.GetSocket()->remote_endpoint().address().to_string();

        }
        LOG_INFO << "Ip: " << ip;

        return false;
    }

};