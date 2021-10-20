#pragma once

#include <FlowUtils/FlowRandom.h>
#include <vector>
#include <bitset>

struct WebSocketFrame {
    bool fin = 1;
    bool rsv1 = 0;
    bool rsv2 = 0;
    bool rsv3 = 0;
    int8_t opcode = 1;
    bool mask = 0;
    uint64_t payload_len;
    unsigned char masking_key[4];
    std::vector<unsigned char> payload_data;

    std::vector<unsigned char> createData() {
        std::vector<unsigned char> rtn;
        unsigned char firstByte = fin;
        firstByte <<= 1;
        firstByte += rsv1;
        firstByte <<= 1;
        firstByte += rsv2;
        firstByte <<= 1;
        firstByte += rsv3;
        firstByte <<= 4;
        firstByte += opcode;
        rtn.emplace_back(firstByte);
        unsigned char secondByte = mask;
        secondByte <<= 7;
        payload_len = payload_data.size();
        if (payload_len < 126) {
            secondByte += payload_len;
            rtn.emplace_back(secondByte);
        } else if (payload_len < 65536) {
            secondByte += 126;
            rtn.emplace_back(secondByte);
            rtn.emplace_back(reinterpret_cast<char *>(&payload_len)[1]);
            rtn.emplace_back(reinterpret_cast<char *>(&payload_len)[0]);
        } else {
            secondByte += 127;
            rtn.emplace_back(secondByte);
            rtn.emplace_back(reinterpret_cast<char *>(&payload_len)[7]);
            rtn.emplace_back(reinterpret_cast<char *>(&payload_len)[6]);
            rtn.emplace_back(reinterpret_cast<char *>(&payload_len)[5]);
            rtn.emplace_back(reinterpret_cast<char *>(&payload_len)[4]);
            rtn.emplace_back(reinterpret_cast<char *>(&payload_len)[3]);
            rtn.emplace_back(reinterpret_cast<char *>(&payload_len)[2]);
            rtn.emplace_back(reinterpret_cast<char *>(&payload_len)[1]);
            rtn.emplace_back(reinterpret_cast<char *>(&payload_len)[0]);
        }

        if (mask) {
            rtn.emplace_back(masking_key[0]);
            rtn.emplace_back(masking_key[1]);
            rtn.emplace_back(masking_key[2]);
            rtn.emplace_back(masking_key[3]);
        }
        rtn.insert(rtn.end(), payload_data.begin(), payload_data.end());

        return rtn;
    }

    bool isConnectionClosed() const {
        return this->opcode == 0x08;
    }

    uint16_t getConnectClosedCode() const {
        if (!isConnectionClosed())
            return 0;

        uint16_t rtn = payload_data[0];
        rtn <<= 8;
        rtn += payload_data[1];
        return rtn;
    }

    static WebSocketFrame readMessage(const std::vector<unsigned char> &buffer, size_t &buffer_pos) {
        WebSocketFrame frame;

        if (buffer.empty()) {
            LOG_WARNING << "Empty message!";
            return frame;
        }

        std::bitset<8> bs(buffer.at(buffer_pos++));
        frame.fin = bs[7];
        frame.rsv1 = bs[6];
        frame.rsv2 = bs[5];
        frame.rsv3 = bs[4];
        frame.opcode = bs[3] * 8 + bs[2] * 4 + bs[1] * 2 + bs[0] * 1;
        bs = std::bitset<8>(buffer.at(buffer_pos++));
        frame.mask = bs[7];
//        frame.payload_len = bs[6] * 64 + bs[5] * 32 + bs[4] * 16 + bs[3] * 8 + bs[2] * 4 + bs[1] * 2 + bs[0] * 1;
        frame.payload_len = 0b01111111 & buffer.at(buffer_pos - 1);
        if (frame.payload_len == 126) {
            frame.payload_len = buffer.at(buffer_pos++);
            frame.payload_len <<= 8;
            frame.payload_len += buffer.at(buffer_pos++);
        }
        if (frame.payload_len == 127) {
            frame.payload_len = buffer.at(buffer_pos++);
            frame.payload_len <<= 8;
            frame.payload_len += buffer.at(buffer_pos++);
            frame.payload_len <<= 8;
            frame.payload_len += buffer.at(buffer_pos++);
            frame.payload_len <<= 8;
            frame.payload_len += buffer.at(buffer_pos++);
            frame.payload_len <<= 8;
            frame.payload_len += buffer.at(buffer_pos++);
            frame.payload_len <<= 8;
            frame.payload_len += buffer.at(buffer_pos++);
            frame.payload_len <<= 8;
            frame.payload_len += buffer.at(buffer_pos++);
            frame.payload_len <<= 8;
            frame.payload_len += buffer.at(buffer_pos++);
        }
        if (frame.mask) {
            frame.masking_key[0] = buffer.at(buffer_pos++);
            frame.masking_key[1] = buffer.at(buffer_pos++);
            frame.masking_key[2] = buffer.at(buffer_pos++);
            frame.masking_key[3] = buffer.at(buffer_pos++);
        }

//        frame.payload_data.insert(frame.payload_data.begin(), buffer.begin() + buffer_pos, buffer.begin() + buffer_pos + frame.payload_len);
        frame.payload_data.assign(buffer.begin() + buffer_pos, buffer.begin() + buffer_pos + frame.payload_len);
        if (frame.mask) {
            frame.xorPayload();
        }
        buffer_pos += frame.payload_len;
        return frame;
    }

    std::vector<unsigned char> createMaskedData() {
        maskFrame();
        return createData();
    }

    void maskFrame() {
        if (this->mask)
            return;

        this->mask = true;
        this->masking_key[0] = FlowRandom::randomChar();
        this->masking_key[1] = FlowRandom::randomChar();
        this->masking_key[2] = FlowRandom::randomChar();
        this->masking_key[3] = FlowRandom::randomChar();
        xorPayload();
    }

    void xorPayload() {
        for (size_t i = 0; i < this->payload_data.size(); ++i) {
            this->payload_data[i] = this->payload_data[i] ^ this->masking_key[i % 4];
        }
    }


};