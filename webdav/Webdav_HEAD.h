#pragma once

#include "../routes/Route.h"
#include <FlowUtils/FlowLog.h>
#include <string>
#include <boost/asio.hpp>
#include "../Socket.h"
#include <FlowUtils/FlowFile.h>

class Webdav_HEAD : public Route {
public:
    explicit Webdav_HEAD(std::string basePath) : basePath(std::move(basePath)), Route(".*", "HEAD") {
    }

    virtual bool Run(Request &request, Response &response, Socket &socket) {
        const auto full_path = FlowFile::combinePath(basePath, request.Path());
        if (FlowFile::exists(full_path)) {
            return FlowAsio::sendOk(socket);
        }
        FlowAsio::sendNotFound(socket);
        return true;

    }

private:
    std::string basePath;
};