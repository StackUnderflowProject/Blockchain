//
// Created by mic on 12/25/24.
//

#include "block.h"

#include <stdexcept>
#include <sstream>
#include <algorithm>

std::string block::get_data() {
    try {
        return {data.begin(), data.end()};
    } catch (const std::exception &e) {
        throw std::runtime_error("Failed to convert data to string" + std::string(e.what()));
    }
}

std::vector<uint8_t> block::hash_block() {
    std::ostringstream oss;

    oss << index << timestamp;

    std::ranges::for_each(data.begin(), data.end(), [&](const uint8_t byte) {oss << byte; });

    std::ranges::for_each(previous_hash.begin(), previous_hash.end(), [&](const uint8_t byte) {oss << byte; });

    oss << difficulty << nonce;

    std::string str = oss.str();

    return {str.begin(), str.end()};
}

std::string block::to_string() {
    std::ostringstream oss;

    oss << index << " | " << timestamp << " | ";

    std::ranges::for_each(data.begin(), data.end(), [&](const uint8_t byte) {oss << byte; });

    oss << " | ";

    std::ranges::for_each(previous_hash.begin(), previous_hash.end(), [&](const uint8_t byte) {oss << byte; });

    oss << " | " << difficulty << " | " << nonce << "\n";

    std::string str = oss.str();

    return oss.str();
}
