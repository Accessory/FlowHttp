#pragma once

#include <string>
#include <vector>
#include "hpack_huffman_table.h"
#include <boost/multiprecision/cpp_int.hpp>

class Huffman {
public:
    static inline std::string encode(const std::vector<unsigned char> &data) {
        return "";
    }

    static inline std::string decode(const std::vector<unsigned char> &huffman_string, bool &success) {
        success = true;

        if (huffman_string.empty()) {
            return "";
        }

        uint8_t state = 0;
        uint8_t flags = 0;
        std::vector<unsigned char> decoded_bytes;


        for (unsigned char input_byte : huffman_string) {
            auto &tablePos = huff_decode_table[state][input_byte >> 4];
            state = tablePos.state;
            flags = tablePos.flags;
            uint8_t output_byte = tablePos.sym;


            if (flags & HUFFMAN_FAIL) {
                success = false;
//                LOG_ERROR << "Invalid Huffman String";
                return "";
            }

            if (flags & HUFFMAN_EMIT_SYMBOL) {
                decoded_bytes.emplace_back(output_byte);
            }

            auto &tablePos2 = huff_decode_table[state][input_byte & 0x0F];
            state = tablePos2.state;
            flags = tablePos2.flags;
            output_byte = tablePos2.sym;

            if (flags & HUFFMAN_FAIL) {
                success = false;
//                LOG_ERROR << "Invalid Huffman String";
                return "";
            }

            if (flags & HUFFMAN_EMIT_SYMBOL) {
                decoded_bytes.emplace_back(output_byte);
            }
        }
        if (!(flags & HUFFMAN_COMPLETE)) {
            success = false;
//            LOG_ERROR << "Incomplete Huffman String";
            return "";
        }
        return std::string(decoded_bytes.begin(), decoded_bytes.end());
    }

private:
    const static inline uint8_t HUFFMAN_COMPLETE = 1;
    const static inline uint8_t HUFFMAN_EMIT_SYMBOL = (1 << 1);
    const static inline uint8_t HUFFMAN_FAIL = (1 << 2);

    static inline size_t calcLength(const std::vector<unsigned char> &data) {
        size_t bits = 0;

        for (auto itr = data.begin(); itr != data.end(); ++itr)
            bits += huff_sym_table[*itr].nbits;

        return (bits + 7) / 8 | 0;
    }

};