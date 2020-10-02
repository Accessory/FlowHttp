#pragma once

#include "Frame.h"

namespace Flow_Http2 {

    enum Setting_Identifier {
        UNDEFINED = 0,
        SETTINGS_HEADER_TABLE_SIZE = 1,
        SETTINGS_ENABLE_PUSH = 2,
        SETTINGS_MAX_CONCURRENT_STREAMS = 3,
        SETTINGS_INITIAL_WINDOW_SIZE = 4,
        SETTINGS_MAX_FRAME_SIZE = 5,
        SETTINGS_MAX_HEADER_LIST_SIZE = 6,
    };

    struct Setting {
        Setting_Identifier Identifier = Setting_Identifier::UNDEFINED;
        uint32_t Value = 0;
    };

    struct Settings_Frame : public Frame, public std::vector<Setting> {
        Settings_Frame()  {
            this->Type = FrameType::SETTINGS;
        }

        virtual size_t Parse(const std::string &data, size_t pos) {
            pos = Frame::Parse(data, pos);

            if(this->Flags == 0x1 && this->Length != 0){
                this->error = FrameError::FRAME_SIZE_ERROR;
                return pos;
            }

            if (this->Type == FrameType::SETTINGS
            && (this->Length % 6) == 0
            && this->Stream_Identifier < 2147483648)
                return this->Parse(data, pos, this->Length);

            this->error = FrameError::PROTOCOL_ERROR;
            return pos;
        }

        size_t Parse(const std::string &data, size_t pos, const size_t length) {
            size_t payloadEnd = pos + length;
            while (pos < payloadEnd) {
                Setting setting;
                uint16_t identifier = (uint) data.at(pos++);
                identifier <<= 8;
                identifier += (uint) data.at(pos++);
                setting.Identifier = (Setting_Identifier) identifier;
                setting.Value += (uint) data.at(pos++);
                setting.Value <<= 8;
                setting.Value += (uint) data.at(pos++);
                setting.Value <<= 8;
                setting.Value += (uint) data.at(pos++);
                setting.Value <<= 8;
                setting.Value += (uint) data.at(pos++);
                this->emplace_back(std::move(setting));
            }

            return pos;
        }
    };
}