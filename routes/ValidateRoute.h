#pragma once

#include "Route.h"
#include <FlowUtils/FlowLog.h>
#include <string>
#include <boost/asio.hpp>
#include "../FlowAsio.h"
#include "../Socket.h"

class ValidateRoute : public Route {
public:
    ValidateRoute() : Route(".*", ".*") {}

    virtual bool Run(Request &request, Response &response, Socket &socket) {
        using namespace FlowAsio;

        if (request.ParserState == HttpParserState::BAD_REQUEST) {
            response.StatusCode = HttpStatusCode::BadRequest;
            write(socket, response.ToString());
            LOG_WARNING << request.Path() << " BadRequest";
            return true;
        }

        auto method = request.Method();

        if (std::find(_methods, _methods + _methods_count, method) == _methods + _methods_count) {
            Response res;
            res.StatusCode = HttpStatusCode::BadRequest;
            write(socket, res.ToString());
            LOG_WARNING << "Bad Request";
            return true;
        }

        if (request.Path().find("..") != string::npos) {
            Response res;
            res.StatusCode = HttpStatusCode::BadRequest;
            write(socket, res.ToString());
            LOG_WARNING << "Bad Request";
            return true;
        }
        return false;
    }

private:
    const static inline std::string _methods[] = {"GET", "POST", "PUT", "DELETE",
                                                  "PATCH", "CONNECT", "HEAD", "OPTIONS", "TRACE"};
    const size_t _methods_count = 9;
};