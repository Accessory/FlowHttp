#pragma once

#include "Route.h"
#include <FlowUtils/FlowLog.h>
#include <string>
#include <boost/asio.hpp>
#include "../FlowAsio.h"
#include <FlowUtils/base64.h>
#include <FlowUtils/FlowString.h>
#include <FlowUtils/FlowUUID.h>
#include "../CookieGod.h"
#include "../manager/SessionManager.h"
#include "../manager/Session.h"

class SessionRoute : public Route {
public:
    SessionRoute() : Route(".*", ".*") {}

    virtual bool Run(Request &request, Response &response, Socket &socket) {
        using namespace FlowAsio;

        auto cookie = request.Header("Cookie");

        auto val = CookieGod::parseCookieValue(cookie, "session");
        shared_ptr<Session> session;

        if(!SessionManager::tryGetSession(FlowUUID::ToUUID(val), session)) {
                session = SessionManager::newSession();
                response.AddCookie("session", FlowUUID::ToString(session->id));
        }

        return false;
    }

};