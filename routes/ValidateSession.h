#pragma once

#include "Route.h"
#include <FlowUtils/FlowLog.h>
#include <string>
#include <boost/asio.hpp>
#include "../FlowAsio.h"
#include <FlowUtils/base64.h>
#include <FlowUtils/FlowString.h>
#include <FlowUtils/FlowUUID.h>
#include "../manager/SessionManager.h"
#include "../manager/Session.h"
#include "../CookieUtil.h"
#include <memory>

class ValidateSession : public Route {
public:
    ValidateSession() : Route(".*", ".*") {
    }

    virtual bool Run(Request &request, Response &response, Socket &socket) {
        using namespace FlowAsio;

        auto cookie = request.Header("Cookie");
        auto val = CookieUtil::parseCookieValue(cookie, "session");

        std::shared_ptr<Session> session;
        if (SessionManager::tryGetSession(FlowUUID::ToUUID(val), session)) {
            if (session->IsAuthorized) {
                return false;
            }
        }
        FlowAsio::sendUnauthorized(socket);
        return true;
    }
};