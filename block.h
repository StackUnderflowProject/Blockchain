//
// Created by mic on 12/25/24.
//

#ifndef BLOCK_H
#define BLOCK_H
#include <cstdint>
#include <ctime>
#include <string>
#include <vector>


class block {
public:
    int index;
    time_t timestamp;
    std::vector<uint8_t> data;
    std::vector<uint8_t> hash;
    std::vector<uint8_t> previous_hash;
    unsigned int difficulty;
    unsigned int nonce;

    block(const int index, const time_t timestamp, const std::vector<uint8_t> &data, const std::vector<uint8_t> &hash,
          const std::vector<uint8_t> &previous_hash, const unsigned int difficulty,
          const unsigned int nonce) : index(index), timestamp(timestamp), data(data), hash(hash),
                                      previous_hash(previous_hash),
                                      difficulty(difficulty), nonce(nonce) {
    }

    std::string get_data();

    std::vector<uint8_t> hash_block();

    std::string to_string();
};


#endif //BLOCK_H