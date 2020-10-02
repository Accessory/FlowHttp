#pragma once

#include "Route.h"
#include <FlowUtils/FlowLog.h>
#include <string>
#include <boost/asio.hpp>
#include "../FlowAsio.h"
#include "../WebSocketFrame.h"
#include "../manager/WebSocketManager.h"
#include <bitset>

class WebSocket : public Route {
public:
    WebSocket() : Route(".*", "GET") {}

private:
    virtual bool Run(Request &request, Response &response, Socket &socket) {
        if (request.Header("Upgrade") == "websocket") {
            WebSocketManager::newSession(request, response, socket);
            return true;
        }
        return false;
    }


};