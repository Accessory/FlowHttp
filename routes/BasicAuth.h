#pragma once

#include "Route.h"
#include <FlowUtils/FlowLog.h>
#include <string>
#include <boost/asio.hpp>
#include "../FlowAsio.h"
#include <FlowUtils/base64.h>

class BasicAuth : public Route {
public:
    BasicAuth() : Route(".*", ".*") {}

private:

    static void writeBasicAuth(Response &response, Socket &socket) {
        response.StatusCode = HttpStatusCode::Unauthorized;
        response.emplace("WWW-Authenticate", R"(Basic realm="User Visible Realm", charset="UTF-8")");
        FlowAsio::write(socket, response.ToString());
    }

    virtual bool Run(Request &request, Response &response, Socket &socket) {
        return serveAuth(request, response, socket);
    }

    static bool serveAuth(Request &request, Response &response, Socket &socket) {

        auto authorization = request.Header("Authorization");
        if (authorization.empty()) {
            writeBasicAuth(response, socket);
            return true;
        }

        auto userKey = Base64::base64_decode(authorization.substr(6));
        if (userKey == "abc:cde")
            return false;

        writeBasicAuth(response, socket);
        return true;
    }

};