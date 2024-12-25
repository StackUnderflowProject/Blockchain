//
// Created by mic on 12/25/24.
//

#ifndef UTILS_H
#define UTILS_H
#include <cstdint>
#include <string>
#include <vector>


class utils {
public:
    static std::vector<uint8_t> hash(const std::vector<uint8_t> &data);

    static std::string vec_to_hex(const std::vector<uint8_t> &data);

    static std::vector<uint8_t> hex_to_vec(const std::string &hex);
};



#endif //UTILS_H
