#pragma once

#include "Route.h"
#include <FlowUtils/FlowLog.h>
#include <string>
#include <boost/asio.hpp>
#include "../FlowAsio.h"
#include "../Socket.h"

class FileNotFound : public Route {
public:
    FileNotFound() : Route(".*", ".*") {}
    virtual bool Run(Request &request, Response &response, Socket &socket) {
            return serve404(request, response, socket);
    }

private:
    static bool serve404(Request &request, Response &response, Socket &socket) {
        using namespace FlowAsio;

        response.StatusCode = HttpStatusCode::NotFound;

        write(socket, response.ToString());

        LOG_WARNING << request.Path() << " not Found";
        return true;
    }

};