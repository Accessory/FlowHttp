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

class Login : public Route {
    const std::string found;
public:
    Login(const std::string& path, const std::string& found = "/") : Route(path, "POST"), found(found) {
    }

    virtual bool Run(Request &request, Response &response, Socket &socket) {
        using namespace FlowAsio;

        std::string username, password;
        username = request.GetParameter("username");
        password = request.GetParameter("password");

        LOG_INFO << username << " " << password;

        if (username == "abc" && password == "cde") {
            shared_ptr<Session> newSession = SessionManager::newSession();
            newSession->IsAuthorized = true;
            response.AddCookie("session", FlowUUID::ToString(newSession->id));
            FlowAsio::sendFound(response, socket, this->found);
            return true;
        }
        FlowAsio::sendUnauthorized(socket);
        return true;
    }
};