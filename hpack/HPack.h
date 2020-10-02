#pragma once

#include <string>
#include <vector>
#include <map>
#include "HPackTable.h"
#include "Huffman.h"
#include <FlowUtils/FlowLog.h>
#include <FlowUtils/FlowSParser.h>
#include <sstream>

class HPack {
public:
    static std::string decode(const std::vector<unsigned char> &data) {
        std::stringstream rtn;
        for (auto itr = data.begin(); itr != data.end(); ++itr) {
            uint8_t i = *itr;
            if (i < 0x80) {
                auto encoded = HPack_Table::encode(i);
                rtn << encoded;
                ++itr;
                uint8_t j = *itr;
                if (j > 0x80) {
                    j -= 0x80;
                    std::vector<unsigned char> tmpData(++itr, itr + j);
                    bool success;
                    auto value = Huffman::decode(tmpData, success);
                    rtn << value << std::endl;
                    itr = itr + j - 1;
                } else {
                    auto len = *itr;
                    std::string value(++itr, itr+len);
                    rtn << value << std::endl;
                    itr = itr + len - 1;

                }
                continue;
            }
            if (i >= 0x80) {
                auto encoded = HPack_Table::encode(i);
                rtn << encoded << std::endl;
            }
        }
        return rtn.str();
    }

    static std::vector<unsigned char> encode(const std::string &data, bool response) {
        using namespace FlowSParser;
        size_t pos = 0;
        std::map<std::string, std::string> headerMap;
        if (!response) {
            auto method = gotoNextNonAlpha(data, pos);
            headerMap.emplace(":method", method);
            gotoNextNonWhite(data, pos);
            auto path = gotoNextNonAlpha(data, pos);
            headerMap.emplace(":path", path);
            gotoNextNonWhite(data, pos);
            auto schema = goTo(data, "/", pos);
            headerMap.emplace(":schema", schema);
            gotoNextNonAlpha(data, pos);
        }
        while (pos < data.size()) {
            gotoNextNonWhite(data, pos);
            if(pos >= data.size())
                break;
            auto key = goTo(data, ": ", pos);
            gotoNextNonWhite(data, ++pos);
            auto value = goToNewLine(data, pos);
            headerMap.emplace(std::move(key), std::move(value));
        }
        return encode(headerMap);
    }

    static std::vector<unsigned char> encode(const std::map<std::string, std::string> &headerMap) {
        using namespace FlowSParser;
        std::vector<unsigned char> rtn;
        for (auto &header : headerMap) {
            auto data = HPack_Table::decode(header.first, header.second);
            rtn.insert(rtn.end(), data.begin(), data.end());
        }
        return rtn;
    }

};
