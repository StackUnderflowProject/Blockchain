//
// Created by mic on 12/25/24.
//

#include "utils.h"

#include <iomanip>
#include <stdexcept>
#include <openssl/evp.h>
#include <openssl/sha.h>

std::vector<uint8_t> utils::hash(const std::vector<uint8_t> &data) {
    const std::string hex = vec_to_hex(data);
    std::vector<uint8_t> hash(SHA256_DIGEST_LENGTH);
    SHA256(reinterpret_cast<const unsigned char *>(hex.c_str()), hex.size(), hash.data());
    return hash;
}

std::string utils::vec_to_hex(const std::vector<uint8_t> &data) {
    std::string hex;
    hex.reserve(data.size() * 2);

    for (const uint8_t byte : data) {
        hex.push_back("0123456789ABCDEF"[byte >> 4]);
        hex.push_back("0123456789ABCDEF"[byte & 0x0F]);
    }

    return hex;
}

std::vector<uint8_t> utils::hex_to_vec(const std::string &hex) {
    std::vector<uint8_t> data;
    data.reserve(hex.size() / 2);

    for (size_t i = 0; i < hex.size(); i += 2) {
        data.push_back(std::stoi(hex.substr(i, 2), nullptr, 16));
    }

    return data;
}

std::string utils::timestamp_to_string(const time_t timestamp) {
    std::ostringstream oss;
    const std::tm *local_time = std::localtime(&timestamp); // Convert to local time
    oss << std::put_time(local_time, "%Y-%m-%d %H:%M:%S"); // Format the time
    return oss.str();
}


