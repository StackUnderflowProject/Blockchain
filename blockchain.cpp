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
        if (std::difftime(last.get_timestamp(), new_block.get_timestamp()) > 60) {
            std::cerr << "Block was not added to blockchain.\n"
                    << "Reject reason: created more than 1 minute earlier than the previous block\n";
            return false;
        }
        if (new_block.get_index() < last.get_index()) {
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
        if (blocks[i].get_previous_hash() != blocks[i - 1].get_hash()) {
            std::cerr << "Blockchain validation failed: Block " << i
                    << " has an invalid PreviousHash.\n";
            return false;
        }
    }
    return true;
}

void blockchain::to_json(nlohmann::json &json, const blockchain &chain) {
    json["difficulty"] = chain.difficulty;
    json["blocks"] = nlohmann::json::array();
    for (const block &b : chain.blocks) {
        nlohmann::json block_json;
        block::to_json(block_json, b);
        json["blocks"].push_back(block_json);
    }
}


void blockchain::from_json(const nlohmann::json &json, blockchain &chain) {
    json.at("difficulty").get_to(chain.difficulty);

    chain.blocks.clear();
    for (const nlohmann::json &block_json : json.at("blocks")) {
        block b;
        block::from_json(block_json, b);
        chain.blocks.push_back(b);
    }
}

unsigned int blockchain::calculate_cumulative_difficulty() {
    return std::accumulate(blocks.begin(), blocks.end(), 0,
                           [](const int acc, const block &b) { return acc + std::pow(2, b.get_difficulty()); });
}
