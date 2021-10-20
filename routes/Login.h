#pragma once

#include "Route.h"
#include <FlowUtils/FlowLog.h>
#include <string>
#include <boost/asio.hpp>
#include "../FlowAsio.h"
#include <FlowUtils/base64.h>
#include <FlowUtils/FlowString.h>
#include <FlowUtils/FlowUUID.h>
#include "../CookieUtil.h"
#include "../manager/SessionManager.h"
#include "../manager/Session.h"
#include "../pwm/PasswordFile.h"

class Login : public Route {
    const std::string found;
    PasswordFile passwordFile;

public:
    Login(const std::string &path,
          const std::string &found = "/",
          const std::string &passwordFile = "pw.txt") :
            Route(path, "POST"),
            found(found),
            passwordFile(passwordFile) {}

    virtual bool Run(Request &request, Response &response, Socket &socket) {
        using namespace FlowAsio;

        std::string username, password;
        username = request.GetParameter("username");
        password = request.GetParameter("password");

        LOG_INFO << username << " " << password;

        if (passwordFile.verifyUser(username, password)) {
            std::shared_ptr<Session> newSession = SessionManager::newSession();
            newSession->IsAuthorized = true;
            response.AddCookie("session", FlowUUID::ToString(newSession->id));
            FlowAsio::sendFound(response, socket, this->found);
            return true;
        }
        FlowAsio::sendUnauthorized(socket);
        return true;
    }
};