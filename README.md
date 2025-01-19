# Distributed Blockchain Miner

A high-performance, distributed blockchain implementation using MPI (Message Passing Interface) for parallel block mining across multiple nodes and OpenSSL for cryptographic operations.

## Features

- Distributed mining using MPI for inter-process communication
- Multi-threaded mining within each worker process
- Automatic difficulty adjustment based on mining performance
- JSON-based blockchain persistence
- Dynamic load balancing across workers
- SHA-256 based proof-of-work mining

## Prerequisites

- CMake (version 3.28 or higher)
- C++20 compatible compiler
- OpenSSL development libraries
- MPI implementation (e.g., OpenMPI or MPICH)

## Building the Project

1. Create a build directory:
```bash
mkdir build
cd build
```

2. Generate build files with CMake:
```bash
cmake ..
```

3. Build the project:
```bash
make
```

## Usage

Run the program using `mpirun` with the following parameters:

```bash
mpirun -np <num_processes> ./blockchain <difficulty> <data> <expected_mine_time> <threads_per_worker> <expected_blocks>
```

### Parameters

- `num_processes`: Total number of MPI processes (1 master + workers)
- `difficulty`: Initial mining difficulty (number of leading zeros required)
- `data`: Input data to be stored in the blockchain
- `expected_mine_time`: Target time (in seconds) for mining a block
- `threads_per_worker`: Number of mining threads per worker process
- `expected_blocks`: Number of blocks to mine before difficulty adjustment

### Example

```bash
mpirun -np 4 ./blockchain 4 "Hello, Blockchain!" 10 2 5
```

This command will:
- Start 4 processes (1 master + 3 workers)
- Set initial difficulty to 4
- Mine blocks containing "Hello, Blockchain!"
- Target 10 seconds per block
- Use 2 threads per worker
- Adjust difficulty every 5 blocks

## Architecture

### Master Process
- Coordinates worker processes
- Manages the blockchain state
- Distributes mining tasks
- Validates and adds mined blocks
- Adjusts mining difficulty
- Saves the final blockchain to disk

### Worker Processes
- Receive mining tasks from master
- Perform multi-threaded block mining
- Submit mined blocks to master
- Handle blockchain updates
- Automatically terminate when all data is processed

## Output

The program creates a `blockchain.json` file containing the complete blockchain data. Each block includes:
- Index
- Timestamp
- Data
- Previous block hash
- Nonce
- Hash

## Performance Considerations

- Increase `num_processes` to utilize more nodes
- Adjust `threads_per_worker` based on CPU cores per node
- Monitor `expected_mine_time` to balance speed and resource usage
- Tune `expected_blocks` for difficulty adjustment frequency

## Technical Details

### Communication Protocol
- TAG_DATA_CHUNK (1): New mining task
- TAG_MINED_BLOCK (2): Completed block
- TAG_TERMINATION (3): Shutdown signal
- TAG_NEW_BLOCK_NOTIFICATION (4): Chain update

### Data Structures
- MiningTask: Serializable structure for task distribution
- Block: Contains block data and mining logic
- Blockchain: Manages the chain of blocks

## Error Handling

The program includes error checking for:
- Invalid command-line arguments
- Block validation failures
- Chain integrity violations
- MPI communication errors

## Limitations

- Input data is split into 64-byte chunks
- Requires at least one worker process
- Memory usage scales with blockchain size
- Network bandwidth may limit scaling
