#pragma once

#include "Route.h"
#include <FlowUtils/FlowLog.h>
#include <string>
#include <boost/asio.hpp>
#include "../FlowAsio.h"
#include "../Socket.h"

class PathCheck : public Route {
public:
    PathCheck() : Route(".*", ".*") {}
    virtual bool Run(Request &request, Response &response, Socket &socket) {
            return pathCheck(request, response, socket);
    }

private:
    static bool pathCheck(Request &request, Response &response, Socket &socket) {
        using namespace FlowAsio;

        if (request.Path().find("..") != string::npos) {
            Response res;
            res.StatusCode = HttpStatusCode::BadRequest;
            write(socket, res.ToString());
            LOG_WARNING << "Bad Request";
            return true;
        }
        return false;
    }

};