//
// Created by mic on 12/25/24.
//

#ifndef BLOCKCHAIN_H
#define BLOCKCHAIN_H
#include <vector>

#include "block.h"


class blockchain {
public:
    unsigned int difficulty;
    std::vector<block> blocks;

    bool add_block(const block &new_block);

    unsigned int calculate_cumulative_difficulty();

    [[nodiscard]] bool validate_chain() const;
};



#endif //BLOCKCHAIN_H
