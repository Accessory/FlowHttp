#pragma once

#include <boost/uuid/uuid.hpp>
#include <vector>
#include "WebSocketSession.h"
#include <FlowUtils/base64.h>
#include <FlowUtils/FlowOpenSSL.h>
#include <FlowUtils/FlowUUID.h>
#include "../FlowAsio.h"
#include <memory>

namespace Http2Manager {
    std::map<boost::uuids::uuid, std::unique_ptr<WebSocketSession>> sessions;

    void deleteSession(boost::uuids::uuid uuid) {
        sessions.erase(uuid);
    }

    void newSession(Request &request, Response &response, Socket &socket) {

    }

};
