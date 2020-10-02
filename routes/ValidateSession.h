#pragma once

#include "Route.h"
#include <FlowUtils/FlowLog.h>
#include <string>
#include <boost/asio.hpp>
#include <FlowHttp/FlowAsio.h>
#include <FlowUtils/base64.h>
#include <FlowUtils/FlowString.h>
#include <FlowUtils/FlowUUID.h>
#include <FlowHttp/CookieGod.h>
#include <FlowHttp/manager/SessionManager.h>
#include <FlowHttp/manager/Session.h>

class ValidateSession : public Route {
public:
    ValidateSession() : Route(".*", ".*") {
    }

    virtual bool Run(Request &request, Response &response, Socket &socket) {
        using namespace FlowAsio;

        auto cookie = request.Header("Cookie");
        auto val = CookieGod::parseCookieValue(cookie, "session");

        shared_ptr<Session> session;
        if (SessionManager::tryGetSession(FlowUUID::ToUUID(val), session)) {
            if (session->IsAuthorized) {
                return false;
            }
        }
        FlowAsio::sendUnauthorized(socket);
        return true;
    }
};