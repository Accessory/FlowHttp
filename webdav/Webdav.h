#pragma once

#include "../routes/Route.h"
#include <FlowUtils/FlowLog.h>
#include <string>
#include <boost/asio.hpp>
#include "../Socket.h"
#include "Webdav_HEAD.h"
#include "Webdav_PROPFIND.h"
#include "../routes/GetRoute.h"
#include "../routes/Router.h"

class Webdav : public Route {
public:
    Webdav(std::string basePath) : basePath(std::move(basePath)), Route(".*", ".*") {
        router.addRoute(std::make_shared<Webdav_HEAD>(this->basePath));
        router.addRoute(std::make_shared<Webdav_PROPFIND>(this->basePath));
        router.addRoute(std::make_shared<GetRoute>(this->basePath));
    }

    virtual bool Run(Request &request, Response &response, Socket &socket) {
        return router.execRoute(request, response, socket);
    }

private:
    Router router;
    std::string basePath;

};