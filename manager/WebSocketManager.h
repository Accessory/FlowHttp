#pragma once

#include <boost/uuid/uuid.hpp>
#include <vector>
#include "WebSocketSession.h"
#include <FlowUtils/base64.h>
#include <FlowUtils/FlowOpenSSL.h>
#include <FlowUtils/FlowUUID.h>
#include "../FlowAsio.h"
#include "../util/WebSocketUtil.h"
#include <memory>

namespace WebSocketManager {
    std::map<boost::uuids::uuid, std::unique_ptr<WebSocketSession>> sessions;

    void deleteSession(boost::uuids::uuid uuid) {
        sessions.erase(uuid);
    }

    void newSession(Request &request, Response &response, Socket &socket) {
        auto key = request.Header("Sec-WebSocket-Key");
        auto accept = WebSocketUtil::createSecWebSocketAccept(key);
        LOG_INFO << "Websocket accept: " << accept;
        response.StatusCode = HttpStatusCode::SwitchingProtocols;
        response.emplace("Upgrade", "websocket");
        response.emplace("Connection", "Upgrade");
        response.emplace("Sec-WebSocket-Accept", accept);
        FlowAsio::write(socket, response.Header());
        boost::uuids::uuid id = FlowUUID::UUID();
        auto session = sessions.emplace(id, make_unique<WebSocketSession>(socket, id));
        session.first->second->onDestroy = [id](WebSocketSession* session) {
            WebSocketManager::deleteSession(id);
        };
        LOG_INFO << "Running Sessions: " << sessions.size();
    }

};
