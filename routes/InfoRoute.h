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

        if(socket.IsAvailable()) {
            std::string ip;
            if (socket.IsSSL()) {
                ip = socket.GetSSLSocket()->lowest_layer().remote_endpoint().address().to_string();
            } else {
                ip = socket.GetSocket()->remote_endpoint().address().to_string();
            }
            LOG_INFO << "Ip: " << ip;
        }

        if (logging::getConfig()->getLevel() == logging::severity::DEBUG ||
            logging::getConfig()->getLevel() == logging::severity::TRACE) {
            LOG_DEBUG << request.ToString();
        }

        return false;
    }

};