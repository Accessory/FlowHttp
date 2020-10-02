#pragma once

namespace Flow_Http2 {

    enum FrameType {
        DATA = 0,
        HEADERS = 1,
        PRIORITY = 2,
        RST_STREAM = 3,
        SETTINGS = 4,
    };

    enum FrameError {
        NO_ERROR = 0x0,
        PROTOCOL_ERROR = 0x1,
        INTERNAL_ERROR = 0x2,
        FLOW_CONTROL_ERROR = 0x3,
        SETTINGS_TIMEOUT = 0x4,
        STREAM_CLOSED = 0x5,
        FRAME_SIZE_ERROR = 0x6,
        REFUSED_STREAM = 0x7,
        CANCEL = 0x8,
        COMPRESSION_ERROR = 0x9,
        CONNECT_ERROR = 0xa,
        ENHANCE_YOUR_CALM = 0xb,
        INADEQUATE_SECURITY = 0xc,
        HTTP_1_1_REQUIRED = 0xd,
    };

    struct Frame {
        uint32_t Length = 0;// 24 bit
        FrameType Type = FrameType::DATA;
        uint8_t Flags = 0;
        bool Reserved = false;
        uint32_t Stream_Identifier = 0;
        vector<unsigned char> Payload;
        FrameError error = FrameError::NO_ERROR;

        std::vector<unsigned char> createData() {
            std::vector<unsigned char> rtn;
            this->Length = Payload.size();
            rtn.emplace_back(this->Length >> 16);
            rtn.emplace_back(this->Length >> 8);
            rtn.emplace_back(this->Length);
            rtn.emplace_back(this->Type);
            rtn.emplace_back(this->Flags);
            rtn.emplace_back(this->Stream_Identifier >> 24);
            rtn.emplace_back(this->Stream_Identifier >> 16);
            rtn.emplace_back(this->Stream_Identifier >> 8);
            rtn.emplace_back(this->Stream_Identifier);
            rtn.insert(rtn.end(), Payload.begin(), Payload.end());
            return rtn;
        };

        virtual size_t Parse(const string &data, size_t pos) {
            this->Length += (uint) data.at(pos++);
            this->Length <<= 8;
            this->Length += (uint) data.at(pos++);
            this->Length <<= 8;
            this->Length += (uint) data.at(pos++);
            this->Type = (FrameType) data.at(pos++);
            this->Flags += (uint) data.at(pos++);
            this->Stream_Identifier += (uint) data.at(pos++);
            this->Stream_Identifier <<= 8;
            this->Stream_Identifier += (uint) data.at(pos++);
            this->Stream_Identifier <<= 8;
            this->Stream_Identifier += (uint) data.at(pos++);
            this->Stream_Identifier <<= 8;
            this->Stream_Identifier += (uint) data.at(pos++);
            return pos;
        }
    };
}