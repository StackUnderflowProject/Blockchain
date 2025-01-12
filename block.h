//
// Created by mic on 12/25/24.
//

#ifndef BLOCK_H
#define BLOCK_H
#include <atomic>
#include <cstdint>
#include <ctime>
#include <string>
#include <vector>

#include "json.hpp"
#include "utils.h"


class block {
    int index{};
    time_t timestamp{};
    std::vector<uint8_t> data;
    std::vector<uint8_t> hash;
    std::vector<uint8_t> previous_hash;
    unsigned int difficulty{};
    uint64_t nonce{};


    block(const int index, const time_t timestamp, const std::vector<uint8_t> &data,
          const std::vector<uint8_t> &previous_hash, const unsigned int difficulty,
          const unsigned int nonce) : index(index), timestamp(timestamp), data(data),
                                      previous_hash(previous_hash),
                                      difficulty(difficulty), nonce(nonce) {
        hash = utils::hash(block_to_vec());
    }

public:
    block() = default;

    [[nodiscard]] std::vector<uint8_t> get_hash() const {
        return hash;
    }

    [[nodiscard]] time_t get_timestamp() const {
        return timestamp;
    }

    [[nodiscard]] std::vector<uint8_t> get_previous_hash() const {
        return previous_hash;
    }

    [[nodiscard]] int get_index() const {
        return index;
    }

    [[nodiscard]] unsigned int get_difficulty() const {
        return difficulty;
    }

    [[nodiscard]] unsigned int get_nonce() const {
        return nonce;
    }

    [[nodiscard]] std::string get_hex() const;

    std::vector<uint8_t> block_to_vec();

    [[nodiscard]] std::string to_string() const;

    static void to_json(nlohmann::json &json, const block& block);

    static void from_json(const nlohmann::json &json, block &b);

    static block mine_block(int index, const std::vector<uint8_t> &data, const std::vector<uint8_t> &previous_hash,
                            unsigned int difficulty, unsigned int thread_id = 0, unsigned int num_threads = 1, const std::atomic_bool &block_minded = false);
};


#endif //BLOCK_H
