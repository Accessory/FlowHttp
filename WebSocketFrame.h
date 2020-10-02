#pragma once

struct WebSocketFrame {
    bool fin = 1;
    bool rsv1 = 0;
    bool rsv2 = 0;
    bool rsv3 = 0;
    int8_t opcode = 1;
    bool mask = 0;
    uint8_t payload_len;
    unsigned char masking_key[4];
    vector<unsigned char> payload_data;

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
        secondByte += payload_len;
        rtn.emplace_back(secondByte);
        if (mask) {
            rtn.insert(rtn.end(), masking_key[0], masking_key[3]);
        }
        rtn.insert(rtn.end(), payload_data.begin(), payload_data.end());

        return rtn;
    }
};