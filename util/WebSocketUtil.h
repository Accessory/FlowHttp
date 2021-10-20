#pragma once

#include <FlowUtils/FlowRandom.h>
#include <FlowUtils/FlowOpenSSL.h>
#include <FlowUtils/base64.h>

namespace WebSocketUtil {
    inline std::string createSecWebSocketKey() {
        const auto key = FlowRandom::getRandomString();
        return Base64::base64_encode(key);
    }

    inline std::string createSecWebSocketAccept(const std::string& secWebSocketKey) {
        const auto key = FlowOpenSSL::sha1(secWebSocketKey + "258EAFA5-E914-47DA-95CA-C5AB0DC85B11");
        return Base64::base64_encode(key);
    }
}