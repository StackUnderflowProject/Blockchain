#include <condition_variable>
#include <iostream>
#include <mutex>
#include "blockchain.h"

std::mutex mtx;
std::condition_variable cv;
std::atomic_bool block_mined(false);
std::atomic_bool genesis_mined(false);

constexpr size_t CHUNK_SIZE = 32;

void mine_block_thread(blockchain &chain, const std::vector<uint8_t> &data, const int index,
                       const unsigned int difficulty, const unsigned int thread_id, const unsigned int num_threads) {
    if (block_mined.load()) return;

    const block new_block = block::mine_block(index, data,
                                              index == 0 ? std::vector<uint8_t>() : chain.blocks.back().get_hash(),
                                              difficulty, thread_id, num_threads, std::ref(block_mined));

    std::lock_guard lock(mtx);
    if (!block_mined.load() && chain.add_block(new_block)) {
        block_mined.store(true);
        cv.notify_all();
        std::cout << "Block mined and added:\n" << new_block.to_string() << std::endl;
    }
}

void mine_genesis_block(blockchain &chain, const std::vector<uint8_t> &data, const unsigned int difficulty) {
    // Use threads to mine the genesis block
    std::cout << "Mining genesis block..." << std::endl;
    while (!genesis_mined.load()) {
        // Generate the genesis block with an empty previous hash (first block)
        block new_block = block::mine_block(0, data, {}, difficulty);

        std::lock_guard lock(mtx);
        if (!genesis_mined.load() && chain.add_block(new_block)) {
            genesis_mined.store(true);
            cv.notify_all(); // Notify all threads that genesis block is mined
            std::cout << "Genesis block mined and added:\n" << new_block.to_string() << std::endl;
            break;
        }
    }
}

std::vector<std::vector<uint8_t>> segment_data(const std::vector<uint8_t> &data, const size_t chunk_size) {
    std::vector<std::vector<uint8_t>> chunks;
    for (size_t i = 0; i < data.size(); i += chunk_size) {
        chunks.emplace_back(data.begin() + i, data.begin() + std::min(i + chunk_size, data.size()));
    }
    return chunks;
}

void adjust_difficulty(blockchain &chain, const int expected_num_blocks, const int expected_mine_time) {
    if (chain.blocks.size() <= expected_num_blocks) return;

    const block &test_block = chain.blocks[chain.blocks.size() - expected_num_blocks - 1];
    const auto expected_time = expected_mine_time * expected_num_blocks;

    if (const auto actual_time = chain.blocks.back().get_timestamp() - test_block.get_timestamp(); actual_time < expected_time / 2) {
        chain.difficulty++;
        std::cout << "Difficulty increased to " << chain.difficulty << ".\n";
    } else if (actual_time > expected_time * 2) {
        if (chain.difficulty > 1) {
            chain.difficulty--;
            std::cout << "Difficulty decreased to " << chain.difficulty << ".\n";
        }
    }
}

int main(const int argc, char *argv[]) {
    if (argc != 5) {
        std::cerr << "Usage: " << argv[0] << " <number of threads> <difficulty> <data> <expected_mine_time>\n";
        return 1;
    }

    blockchain chain;
    chain.difficulty = std::stoul(argv[2]);
    std::string data_str = argv[3];
    const std::vector<uint8_t> data(data_str.begin(), data_str.end());

    const int num_threads = std::stoi(argv[1]);

    const int expected_mine_time = std::stoi(argv[4]);

    std::vector<std::thread> threads;

    std::vector<std::vector<uint8_t>> chunks = segment_data(data, CHUNK_SIZE);

    std::string genesis_data = "genesis";
    // Start mining threads for the genesis block
    mine_genesis_block(std::ref(chain), {genesis_data.begin(),genesis_data.end()}, chain.difficulty);

    // chain.difficulty = 3;

    // Mine each data chunk into separate blocks
    for (size_t i = 0; i < chunks.size(); ++i) {
        std::cout << "Mining block " << i + 1 << " for data chunk...\n";

        block_mined.store(false); // Reset the block mined flag for the next block

        // Start threads for the current block
        for (unsigned int j = 0; j < num_threads; ++j) {
            threads.emplace_back(mine_block_thread, std::ref(chain), chunks[i], chain.blocks.size(),
                                 chain.difficulty, j, num_threads);
        }

        // Wait until a block is mined
        {
            std::unique_lock lock(mtx);
            cv.wait(lock, [] { return block_mined.load(); });
        }

        // Join threads
        for (auto &t : threads) {
            t.join();
        }
        threads.clear(); // Clear the threads vector for the next block

        if (constexpr int expected_num_blocks = 2; chain.blocks.size() % expected_num_blocks == 0) {
            adjust_difficulty(std::ref(chain), expected_num_blocks, expected_mine_time);
        }
    }

    // Validate the blockchain
    if (chain.validate_chain()) {
        std::cout << "Blockchain is valid!\n";
    } else {
        std::cerr << "Blockchain validation failed!\n";
    }


    return 0;
}
