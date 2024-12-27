//
// Created by mic on 12/25/24.
//

#include "block.h"

#include <stdexcept>
#include <sstream>
#include <algorithm>
#include <atomic>
#include <iostream>
#include <openssl/sha.h>

#include "utils.h"

std::string block::get_hex() const {
    try {
        return utils::vec_to_hex(data);
    } catch (const std::exception &e) {
        throw std::runtime_error("Failed to convert data to string" + std::string(e.what()));
    }
}

std::vector<uint8_t> block::block_to_vec() {
    std::ostringstream oss;

    oss << index << timestamp;

    std::ranges::for_each(data.begin(), data.end(), [&](const uint8_t byte) { oss << byte; });

    std::ranges::for_each(previous_hash.begin(), previous_hash.end(), [&](const uint8_t byte) { oss << byte; });

    oss << difficulty << nonce;

    std::string str = oss.str();

    return {str.begin(), str.end()};
}

std::string block::to_string() const {
    std::ostringstream oss;
    oss << "Index: " << index << "\n"
            << "Timestamp: " << utils::timestamp_to_string(timestamp) << "\n"
            << "Data: " << utils::vec_to_hex(data) << "\n"
            << "Hash: " << utils::vec_to_hex(hash) << "\n"
            << "Previous Hash: " << utils::vec_to_hex(previous_hash) << "\n"
            << "Difficulty: " << difficulty << "\n"
            << "Nonce: " << nonce << "\n";
    return oss.str();
}

block block::mine_block(const int index, const std::vector<uint8_t> &data,
                        const std::vector<uint8_t> &previous_hash, const unsigned int difficulty,
                        const unsigned int thread_id, const unsigned int num_threads,
                        const std::atomic_bool &block_minded) {
    const time_t timestamp = time(nullptr);

    block new_block(index, timestamp, data, previous_hash, difficulty, thread_id);

    const std::vector<uint8_t> target(difficulty, 0x00);

    do {
        new_block.nonce += num_threads;
        new_block.hash = utils::hash(new_block.block_to_vec());

        if (block_minded.load()) {
            break;
        }

    } while (std::vector(new_block.hash.begin(), new_block.hash.begin() + difficulty) != target);

    return new_block;
}
