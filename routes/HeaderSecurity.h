#pragma once

#include "Route.h"
#include <FlowUtils/FlowLog.h>
#include <string>
#include "../FlowAsio.h"
#include "../Socket.h"

class HeaderSecurity : public Route {
public:
    HeaderSecurity() : Route(".*", ".*") {}

    virtual bool Run(Request &request, Response &response, Socket &socket) {
        response.emplace("X-XSS-Protection", "1");
        response.emplace("X-Content-Type-Options", "nosniff");
        response.emplace("X-Frame-Options", "SAMEORIGIN");
        response.emplace("Strict-Transport-Security", "max-age=2592000");
        response.emplace("Referrer-Policy", "no-referrer");
        return false;
    }
};