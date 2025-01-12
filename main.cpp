#include <mpi.h>
#include <fstream>
#include <iostream>
#include <mutex>
#include <string>
#include <thread>

#include "json.hpp"
#include "blockchain.h"
#include "block.h"

constexpr size_t CHUNK_SIZE = 64;
constexpr int MASTER_RANK = 0;
constexpr int TAG_DATA_CHUNK = 1;
constexpr int TAG_MINED_BLOCK = 2;
constexpr int TAG_TERMINATION = 3;
constexpr int TAG_NEW_BLOCK_NOTIFICATION = 4;

// Structure to hold mining task data
struct MiningTask {
    int index{};
    unsigned int difficulty{};
    std::vector<uint8_t> data;
    std::vector<uint8_t> previous_hash;

    // Serialize the mining task for MPI transmission
    [[nodiscard]] std::vector<uint8_t> serialize() const {
        // Convert to JSON for easier serialization
        nlohmann::json j;
        j["index"] = index;
        j["difficulty"] = difficulty;
        j["data"] = data;
        j["previous_hash"] = previous_hash;

        // Convert JSON to string and then to bytes
        std::string json_str = j.dump();
        return {json_str.begin(), json_str.end()};
    }

    // Deserialize from MPI transmission
    static MiningTask deserialize(const std::vector<uint8_t> &buffer) {
        std::string json_str(buffer.begin(), buffer.end());
        nlohmann::json j = nlohmann::json::parse(json_str);

        MiningTask task;
        task.index = j["index"].get<int>();
        task.difficulty = j["difficulty"].get<unsigned int>();
        task.data = j["data"].get<std::vector<uint8_t> >();
        task.previous_hash = j["previous_hash"].get<std::vector<uint8_t> >();

        return task;
    }
};

std::vector<std::vector<uint8_t> > segment_data(const std::vector<uint8_t> &data, const size_t chunk_size) {
    std::vector<std::vector<uint8_t> > chunks;
    for (size_t i = 0; i < data.size(); i += chunk_size) {
        chunks.emplace_back(data.begin() + i, data.begin() + std::min(i + chunk_size, data.size()));
    }
    return chunks;
}

void adjust_difficulty(blockchain &chain, const int expected_num_blocks, const int expected_mine_time) {
    if (chain.blocks.size() <= expected_num_blocks) return;

    const block &test_block = chain.blocks[chain.blocks.size() - expected_num_blocks - 1];
    const auto expected_time = expected_mine_time * expected_num_blocks;

    if (const auto actual_time = chain.blocks.back().get_timestamp() - test_block.get_timestamp();
        actual_time < expected_time / 2) {
        chain.difficulty++;
        // std::cout << "Difficulty increased to " << chain.difficulty << ".\n";
    } else if (actual_time > expected_time * 2) {
        if (chain.difficulty > 1) {
            chain.difficulty--;
            // std::cout << "Difficulty decreased to " << chain.difficulty << ".\n";
        }
    }
}

void notify_all_workers(const int num_processes, const block &mined_block) {
    nlohmann::json json;
    block::to_json(json, mined_block);
    std::string json_str = json.dump();
    const std::vector<uint8_t> serialized(json_str.begin(), json_str.end());

    for (int i = 1; i < num_processes; ++i) {
        MPI_Send(serialized.data(), serialized.size(), MPI_BYTE, i,
                 TAG_NEW_BLOCK_NOTIFICATION, MPI_COMM_WORLD);
    }
}

void master_process(const std::vector<uint8_t> &input_data, const unsigned int difficulty,
                    const int num_processes, const int expected_mine_time, const int expected_num_blocks) {
    blockchain chain;
    chain.difficulty = difficulty;

    // Mine genesis block on master
    std::string genesis_data = "genesis";
    std::vector<uint8_t> genesis_vector(genesis_data.begin(), genesis_data.end());
    if (block genesis_block = block::mine_block(0, genesis_vector, {}, difficulty); !chain.add_block(genesis_block)) {
        std::cerr << "Failed to add genesis block!\n";
        MPI_Abort(MPI_COMM_WORLD, 1);
        return;
    }

    // Segment data into chunks
    auto chunks = segment_data(input_data, CHUNK_SIZE);
    int current_chunk = 0;
    int active_workers = num_processes - 1;

    // Broadcast the initial chunk to all workers
    if (current_chunk < chunks.size()) {
        auto current_hash = chain.blocks.back().get_hash();
        MiningTask task{
            .index = static_cast<int>(chain.blocks.size()),
            .difficulty = chain.difficulty,
            .data = chunks[current_chunk],
            .previous_hash = current_hash
        };

        auto serialized = task.serialize();
        for (int rank = 1; rank < num_processes; ++rank) {
            MPI_Send(serialized.data(), serialized.size(), MPI_BYTE, rank,
                     TAG_DATA_CHUNK, MPI_COMM_WORLD);
        }
    }

    // Process results and broadcast updates
    while (active_workers > 0) {
        MPI_Status status;
        std::vector<uint8_t> buffer;
        int count;

        MPI_Probe(MPI_ANY_SOURCE, TAG_MINED_BLOCK, MPI_COMM_WORLD, &status);
        MPI_Get_count(&status, MPI_BYTE, &count);
        buffer.resize(count);

        MPI_Recv(buffer.data(), count, MPI_BYTE, status.MPI_SOURCE,
                 TAG_MINED_BLOCK, MPI_COMM_WORLD, &status);

        // Deserialize the block
        std::string json_str(buffer.begin(), buffer.end());
        nlohmann::json j = nlohmann::json::parse(json_str);
        block new_block;
        block::from_json(j, new_block);

        // Verify the mined block
        if (new_block.get_previous_hash() == chain.blocks.back().get_hash()) {
            if (chain.add_block(new_block)) {
                // std::cout << "Block " << new_block.get_index() << " added to chain\n";

                // Adjust difficulty if needed
                if (chain.blocks.size() % expected_num_blocks == 0) {
                    adjust_difficulty(chain, expected_num_blocks, expected_mine_time);
                }

                // Broadcast the new block to all workers
                notify_all_workers(num_processes, new_block);

                // Prepare the next chunk
                current_chunk++;
                if (current_chunk < chunks.size()) {
                    auto current_hash = chain.blocks.back().get_hash();
                    MiningTask task{
                        .index = static_cast<int>(chain.blocks.size()),
                        .difficulty = chain.difficulty,
                        .data = chunks[current_chunk],
                        .previous_hash = current_hash
                    };

                    auto serialized = task.serialize();
                    for (int rank = 1; rank < num_processes; ++rank) {
                        MPI_Send(serialized.data(), serialized.size(), MPI_BYTE, rank,
                                 TAG_DATA_CHUNK, MPI_COMM_WORLD);
                    }
                } else {
                    // No more chunks: Terminate all workers
                    // std::cout << "All chunks processed. Terminating workers...\n";
                    for (int rank = 1; rank < num_processes; ++rank) {
                        MPI_Send(nullptr, 0, MPI_BYTE, rank, TAG_TERMINATION, MPI_COMM_WORLD);
                    }
                    active_workers = 0; // End processing loop
                }
            } else {
                std::cerr << "Failed to add block " << new_block.get_index() << " to chain\n";
            }
        }
    }

    // Validate the final blockchain
    if (chain.validate_chain()) {
        // std::cout << "Blockchain is valid!\n";
        blockchain::save_blockchain_to_file(chain, "blockchain.json");
    } else {
        std::cerr << "Blockchain validation failed!\n";
    }


    // log_performance(num_processes, {global_stats});
}

void worker_process(const int rank, const int size, const int num_threads = 1) {
    while (true) {
        MPI_Status status;
        std::vector<uint8_t> buffer; // Adjust size as needed
        int count;

        // Check for incoming messages without blocking
        int flag;
        MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &flag, &status);

        if (!flag) continue;

        if (status.MPI_TAG == TAG_TERMINATION) {
            MPI_Recv(nullptr, 0, MPI_BYTE, MASTER_RANK, TAG_TERMINATION,
                     MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            // std::cout << "Worker " << rank << " terminating\n";
            break;
        }

        MPI_Get_count(&status, MPI_BYTE, &count);
        buffer.resize(count);

        // Set up multithreaded mining
        std::atomic_bool block_mined(false);
        std::vector<std::thread> mining_threads;

        if (status.MPI_TAG == TAG_NEW_BLOCK_NOTIFICATION) {
            MPI_Recv(buffer.data(), count, MPI_BYTE, status.MPI_SOURCE,
                     TAG_NEW_BLOCK_NOTIFICATION, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

            // Stop current mining operation
            block_mined.store(true);

            // Wait for mining threads to finish
            for (auto &thread: mining_threads) {
                if (thread.joinable()) {
                    thread.join();
                }
            }

            // Clear mining threads vector
            mining_threads.clear();

            // Continue the loop to receive new work
            continue;
        }

        if (status.MPI_TAG == TAG_DATA_CHUNK) {
            std::mutex mtx;
            block mined_block;
            MPI_Recv(buffer.data(), count, MPI_BYTE, MASTER_RANK, TAG_DATA_CHUNK,
                     MPI_COMM_WORLD, MPI_STATUS_IGNORE);

            auto [index, difficulty, data, previous_hash] = MiningTask::deserialize(buffer);

            // Launch mining threads
            for (int thread_id = 0; thread_id < num_threads; ++thread_id) {
                mining_threads.emplace_back([&](const int tid) {
                    // Calculate the global thread ID based on rank and local thread ID
                    const int global_thread_id = (rank - 1) * num_threads + tid;
                    const block thread_block = block::mine_block(
                        index,
                        data,
                        previous_hash,
                        difficulty,
                        global_thread_id,
                        num_threads * (size - 1), // Total threads across all processes
                        std::ref(block_mined)
                    );

                    if (!block_mined.load()) {
                        std::lock_guard lock(mtx);
                        if (!block_mined.load()) {
                            mined_block = thread_block;
                            block_mined.store(true);
                        }
                    }
                }, thread_id);
            }

            // Wait for all threads to complete
            for (auto &thread: mining_threads) {
                thread.join();
            }

            // Serialize the block using the JSON functions
            nlohmann::json j;
            block::to_json(j, mined_block);
            std::string json_str = j.dump();
            std::vector<uint8_t> serialized(json_str.begin(), json_str.end());

            // Send the mined block back to master
            MPI_Send(serialized.data(), serialized.size(), MPI_BYTE,
                     MASTER_RANK, TAG_MINED_BLOCK, MPI_COMM_WORLD);
        }
    }
}

int main(int argc, char *argv[]) {
    MPI_Init(&argc, &argv);

    int rank, num_processes;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &num_processes);

    if (argc != 6 && rank == MASTER_RANK) {
        std::cerr << "Usage: " << argv[0] <<
                " <difficulty> <data> <expected_mine_time> <number_of_threads_per_worker> <expected_number_of_blocks>\n";
        MPI_Abort(MPI_COMM_WORLD, 1);
        return 1;
    }

    if (rank == MASTER_RANK) {
        const unsigned int difficulty = std::stoul(argv[1]);
        std::string data_str = argv[2];
        const std::vector<uint8_t> data(data_str.begin(), data_str.end());
        const int expected_mine_time = std::stoi(argv[3]);
        const int expected_number_of_blocks = argc == 6 ? std::stoi(argv[5]) : 5;
        master_process(data, difficulty, num_processes, expected_mine_time, expected_number_of_blocks);
    } else {
        const int num_threads = std::stoi(argv[4]);
        worker_process(rank, num_processes, num_threads);
    }

    MPI_Finalize();
    return 0;
}
