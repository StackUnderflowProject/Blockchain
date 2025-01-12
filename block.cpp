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

    std::ranges::for_each(data, [&](const uint8_t byte) { oss << byte; });

    std::ranges::for_each(previous_hash, [&](const uint8_t byte) { oss << byte; });

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

void block::to_json(nlohmann::json &json, const block &block) {
    json = nlohmann::json{
        {"index", block.index},
        {"timestamp", block.timestamp},
        {"data", block.data},
        {"hash", block.hash},
        {"previous_hash", block.previous_hash},
        {"difficulty", block.difficulty},
        {"nonce", block.nonce}
    };
}

void block::from_json(const nlohmann::json &json, block &b) {
    json.at("index").get_to(b.index);
    json.at("timestamp").get_to(b.timestamp);
    json.at("data").get_to(b.data);
    json.at("hash").get_to(b.hash);
    json.at("previous_hash").get_to(b.previous_hash);
    json.at("difficulty").get_to(b.difficulty);
    json.at("nonce").get_to(b.nonce);
}


block block::mine_block(const int index, const std::vector<uint8_t> &data,
                        const std::vector<uint8_t> &previous_hash, const unsigned int difficulty,
                        const unsigned int thread_id, const unsigned int num_threads,
                        const std::atomic_bool &block_minded) {
    const time_t timestamp = time(nullptr);
    block new_block(index, timestamp, data, previous_hash, difficulty, thread_id);
    const std::string target(difficulty, '0');

    // const auto start_time = std::chrono::steady_clock::now();
    uint64_t number_of_hashes = 0;
    do {
        new_block.nonce += num_threads;
        new_block.hash = utils::hash(new_block.block_to_vec());
        number_of_hashes++;

        if (block_minded.load()) {
            break;
        }
    } while (utils::vec_to_hex(new_block.hash).substr(0, difficulty) != target);

    // const auto duration = static_cast<double>(std::chrono::duration_cast<std::chrono::microseconds>(
    //     std::chrono::steady_clock::now() - start_time).count());
    // std::stringstream ss;
    // ss << thread_id << " | " << index << " | " << number_of_hashes << " | " << duration << " | " << number_of_hashes * 1'000'000 / duration << '\n';
    // std::cout << ss.str();

    return new_block;
}
