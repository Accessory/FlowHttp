#pragma once

#include "Route.h"
#include <FlowUtils/FlowLog.h>
#include <string>
#include <vector>
#include <boost/asio.hpp>
#include "../FlowAsio.h"
#include "../Socket.h"
#include <FlowUtils/FlowString.h>
#include <FlowUtils/UrlEscape.h>

class ValidateRoute : public Route {
public:
    ValidateRoute(std::set<std::string> allowedMethods) : Route(".*", ".*"),
                                                          _allowedMethods(std::move(allowedMethods)) {}

    ValidateRoute() : Route(".*", ".*") {}

    virtual bool Run(Request &request, Response &response, Socket &socket) {
        using namespace FlowAsio;

        if (request.ParserState == HttpParserState::BAD_REQUEST) {
            response.StatusCode = HttpStatusCode::BadRequest;
            FlowAsio::sendBadRequest(socket);
            request.CloseConnection();
            LOG_WARNING << request.Path() << " Validation - Bad Request - reason: Parsing error!" << std::endl
                        << request.ToString();
            return true;
        }

        auto method = request.Method();

        if (!_allowedMethods.empty()) {
            if (_allowedMethods.count(method) == 0) {
                FlowAsio::sendMethodNotAllowed(socket);
                request.CloseConnection();
                LOG_WARNING << "Validation - Bad Request - reason: Method type not allowed: " << method;
                return true;
            }
        } else if (std::find(_methods, _methods + _methods_count, method) == _methods + _methods_count) {
            FlowAsio::sendBadRequest(socket);
            LOG_WARNING << "Bad Request";
            return true;
        }
        const auto path = UrlEscape::DecodeUrl(request.Path());

        if (path.find("/../") != std::string::npos) {
            FlowAsio::sendBadRequest(socket);
            request.CloseConnection();
            LOG_WARNING << "Validation - Bad Request - reason: /../";
            return true;
        }
        return false;
    }

private:
    const static inline std::string _methods[] = {"GET", "POST", "PUT", "PATCH", "DELETE", "OPTION", "HEAD", "CONNECT",
                                                  "TRACE", "COPY", "LOCK", "MKCOL", "MOVE", "PROPFIND", "PROPPATCH",
                                                  "UNLOCK"};
    const size_t _methods_count = 16;
    const std::set<std::string> _allowedMethods;
};