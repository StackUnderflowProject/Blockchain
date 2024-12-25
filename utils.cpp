//
// Created by mic on 12/25/24.
//

#include "utils.h"

#include <stdexcept>
#include <openssl/evp.h>

std::vector<uint8_t> utils::hash(const std::vector<uint8_t> &data) {
    std::vector<uint8_t> hash(EVP_MAX_MD_SIZE);
    unsigned int length = 0;

    EVP_MD_CTX *ctx = EVP_MD_CTX_new();
    if (ctx == nullptr) {
        throw std::runtime_error("Failed to create EVP_MD_CTX");
    }

    if (EVP_DigestInit_ex(ctx, EVP_sha256(), nullptr) != 1 ||
        EVP_DigestUpdate(ctx, data.data(), data.size()) != 1 ||
        EVP_DigestFinal_ex(ctx, hash.data(), &length) != 1) {
        EVP_MD_CTX_free(ctx);
        throw std::runtime_error("Failed to compute SHA-256 hash");
        }

    EVP_MD_CTX_free(ctx);
    hash.resize(length);
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

