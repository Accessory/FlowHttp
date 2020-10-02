#pragma once

#include <string>
#include <vector>
#include <set>

struct HPack_Entry {

    HPack_Entry(uint8_t index, const std::string &key, const std::string &value) : index(index), key(key),
                                                                                   value(value) {}

    bool operator<(const HPack_Entry &rhs) const {
        return rhs.index < this->index;
    }

    bool operator<(const uint8_t &rhs) const {
        return this->index == rhs;
    }

    uint8_t index;
    std::string key;
    std::string value;
};

class HPack_Table {
private:
    static inline std::vector<HPack_Entry> table = {
            {1,  ":authority",                  ""},
            {2,  ":method",                     "GET"},
            {3,  ":method",                     "POST"},
            {4,  ":path",                       "/"},
            {5,  ":path",                       "/index.html"},
            {6,  ":scheme",                     "http"},
            {7,  ":scheme",                     "https"},
            {8,  ":status",                     "200"},
            {9,  ":status",                     "204"},
            {10, ":status",                     "206"},
            {11, ":status",                     "304"},
            {12, ":status",                     "400"},
            {13, ":status",                     "404"},
            {14, ":status",                     "500"},
            {15, "accept-charset",              ""},
            {16, "accept-encoding",             "gzip, deflate"},
            {17, "accept-language",             ""},
            {18, "accept-ranges",               ""},
            {19, "accept",                      ""},
            {20, "access-control-allow-origin", ""},
            {21, "age",                         ""},
            {22, "allow",                       ""},
            {23, "authorization",               ""},
            {24, "cache-control",               ""},
            {25, "content-disposition",         ""},
            {26, "content-encoding",            ""},
            {27, "content-language",            ""},
            {28, "content-length",              ""},
            {29, "content-location",            ""},
            {30, "content-range",               ""},
            {31, "content-type",                ""},
            {32, "cookie",                      ""},
            {33, "date",                        ""},
            {34, "etag",                        ""},
            {35, "expect",                      ""},
            {36, "expires",                     ""},
            {37, "from",                        ""},
            {38, "host",                        ""},
            {39, "if-match",                    ""},
            {40, "if-modified-since",           ""},
            {41, "if-none-match",               ""},
            {42, "if-range",                    ""},
            {43, "if-unmodified-since",         ""},
            {44, "last-modified",               ""},
            {45, "link",                        ""},
            {46, "location",                    ""},
            {47, "max-forwards",                ""},
            {48, "proxy-authenticate",          ""},
            {49, "proxy-authorization",         ""},
            {50, "range",                       ""},
            {51, "referer",                     ""},
            {52, "refresh",                     ""},
            {53, "retry-after",                 ""},
            {54, "server",                      ""},
            {55, "set-cookie",                  ""},
            {56, "strict-transport-security",   ""},
            {57, "transfer-encoding",           ""},
            {58, "user-agent",                  ""},
            {59, "vary",                        ""},
            {60, "via",                         ""},
            {61, "www-authenticate",            ""},
    };
public:
    static std::string toHeader(const uint8_t &index) {
        auto &entry = table.at(index - 1);
        return entry.key + ": " + entry.value;
    }

    static std::string encode(uint8_t i) {

        if (i > 0x40 && i < 0x80) {
            i -= 0x41;
        } else if (i > 0x80) {
            i -= 0x81;
            auto &entry = table.at(i);
            return entry.key + ": " + entry.value;
        }

        return table.at(i).key + ": ";
    }

    static std::vector<unsigned char> decode(const std::string &key, const std::string &value) {
        std::vector<unsigned char> rtn;
        uint8_t idx = 0;
        bool needValue = true;
        for (auto &entry : table) {
            if (idx != 0 && entry.key != key) {
                break;
            }
            if (idx == 0 && entry.key == key) {
                idx = entry.index;
            }

            if (idx != 0 && entry.value == value) {
                idx = entry.index;
                needValue = false;
                break;
            }
        }

        uint8_t mark = needValue ? 0x40 : 0x80;
        uint8_t byte = mark + idx;

        rtn.emplace_back(byte);

        if (needValue) {
            uint8_t length = value.size();
            rtn.emplace_back(length);
            for (unsigned char c : value) {
                rtn.emplace_back(c);
            }
        }

        return rtn;
    }
};
