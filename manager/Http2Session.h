#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-noreturn"
#pragma once

#include "../Socket.h"
#include "../FlowAsio.h"
#include <FlowUtils/FlowLog.h>
#include <thread>
#include "WebSocketManager.h"

class Http2Session {
public:
    Http2Session(const Socket &socket, const boost::uuids::uuid& id) : socket(socket), Id(id) {
        std::thread([&] {
            try {
                while (true) {
                    std::vector<unsigned char> buffer;
                    boost::system::error_code error;
                    auto read_size = FlowAsio::readToVector(this->socket, buffer, error, BUFFER_SIZE);
                    if (read_size == 0) {
                        LOG_INFO << error.message();
                        onDestroy(this);
                        return;
                    }
                    auto msg = readMessage(buffer);
                    if(msg.opcode == 8){
                        onDestroy(this);
                        return;
                    }
                    auto msg_str = string(msg.payload_data.begin(), msg.payload_data.end());
                    LOG_INFO << msg_str;
                    sendMessage(msg_str);
                }
            } catch (const std::exception &ignore) {
                onDestroy(this);
            }
        }).detach();
    }

    ~Http2Session() {
    }

    const boost::uuids::uuid Id;
    Socket socket;
    std::function<void(Http2Session* session)> onDestroy;
private:

    const size_t BUFFER_SIZE = 1024;

    void sendMessage(const std::string& text) {
        WebSocketFrame frame;
        frame.payload_data.assign(text.begin(), text.end());
        frame.payload_len = (unsigned char) text.size();

        FlowAsio::write(this->socket, frame.createData());
    }

    WebSocketFrame readMessage(std::vector<unsigned char> buffer) {
        size_t buffer_pos = 0;
        WebSocketFrame frame;
        bitset<8> bs(buffer.at(buffer_pos++));
        frame.fin = bs[7];
        frame.rsv1 = bs[6];
        frame.rsv2 = bs[5];
        frame.rsv3 = bs[4];
        frame.opcode = bs[3] * 8 + bs[2] * 4 + bs[1] * 2 + bs[0] * 1;
        bs = bitset<8>(buffer.at(buffer_pos++));
        frame.mask = bs[7];
        frame.payload_len = bs[6] * 64 + bs[5] * 32 + bs[4] * 16 + bs[3] * 8 + bs[2] * 4 + bs[1] * 2 + bs[0] * 1;
        if (frame.mask) {
            frame.masking_key[0] = buffer.at(buffer_pos++);
            frame.masking_key[1] = buffer.at(buffer_pos++);
            frame.masking_key[2] = buffer.at(buffer_pos++);
            frame.masking_key[3] = buffer.at(buffer_pos++);
        }

        frame.payload_data.assign(buffer.begin() + buffer_pos, buffer.begin() + buffer_pos + frame.payload_len);
        if (frame.mask) {
            for (size_t i = 0; i < frame.payload_data.size(); ++i) {
                frame.payload_data[i] = frame.payload_data[i] ^ frame.masking_key[i % 4];
            }
        }

        return frame;
    }
};

#pragma clang diagnostic pop