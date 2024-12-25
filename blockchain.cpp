//
// Created by mic on 12/25/24.
//

#include "blockchain.h"

#include <cmath>
#include <iostream>
#include <numeric>

bool blockchain::add_block(const block &new_block) {
    if (!blocks.empty()) {
        const block &last = blocks.back();
        if (std::difftime(last.timestamp, new_block.timestamp) > 60) {
            std::cerr << "Block was not added to blockchain.\n"
                << "Reject reason: created more than 1 minute earlier than the previous block\n";
            return false;
        }
        if (new_block.index < last.index) {
            throw std::runtime_error("block index is invalid");
        }
    }

    blocks.emplace_back(new_block);
    if (validate_chain()) return true;

    blocks.pop_back();
    return false;
}

bool blockchain::validate_chain() const {
    for (size_t i = 1; i < blocks.size(); ++i) {
        if (blocks[i].previous_hash != blocks[i - 1].hash) {
            std::cerr << "Blockchain validation failed: Block " << i
                << " has an invalid PreviousHash.\n";
            return false;
        }
    }
    return true;
}

unsigned int blockchain::calculate_cumulative_difficulty() {
    return std::accumulate(blocks.begin(), blocks.end(), 0,
        [](const int acc, const block &b) { return acc + std::pow(2, b.difficulty); });
}
