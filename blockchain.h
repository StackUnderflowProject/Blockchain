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

    static void from_json(const nlohmann::json &json, blockchain &chain);

    static void to_json(nlohmann::json &json, const blockchain &chain);

    static bool save_blockchain_to_file(const blockchain &chain, const std::string &filename) {
        if (std::ofstream file(filename); file.is_open()) {
            nlohmann::json json;
            to_json(json, chain);
            file << json.dump(4);
            file.close();
            std::cout << "Blockchain saved to " << filename << ".\n";
            return true;
        }

        std::cerr << "Failed to save blockchain to " << filename << ".\n";
        return false;
    }

    static bool load_blockchain_from_file(blockchain &chain, const std::string &filename) {
        if (std::ifstream file(filename); file.is_open()) {
            nlohmann::json json;
            file >> json;
            file.close();
            from_json(json, chain);
            std::cout << "Blockchain loaded from " << filename << ".\n";
            if (!chain.validate_chain()) {
                std::cerr << "Loaded blockchain is invalid!\n";
                return false;
            }
            return true;
        }

        std::cerr << "Failed to load blockchain from " << filename << ".\n";
        return false;
    }
};



#endif //BLOCKCHAIN_H
