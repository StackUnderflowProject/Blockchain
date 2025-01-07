#!/bin/bash

# Assign arguments to variables
PROGRAM_PATH="/home/mic/CLionProjects/blockchain/cmake-build-release/blockchain"
NUM_OF_PROCESSES=4
DIFFICULTY=4
INPUT_FILE="/home/mic/CLionProjects/blockchain/data.json"
EXPECTED_TIME=10
NUM_OF_PROCESSES_PER_WORKER=2
EXPECTED_NUM_OF_BLOCKS=3

# Check if the input file exists
if [ ! -f "$INPUT_FILE" ]; then
  echo "Error: Input file '$INPUT_FILE' not found."
  exit 1
fi

# Read the contents of the input file
DATA=$(cat "$INPUT_FILE")

CURRENT_TIME=$(date)
echo "Time: $(date)"

# Run the blockchain program with the file contents as the data argument
mpirun -np "$NUM_OF_PROCESSES" "$PROGRAM_PATH" "$DIFFICULTY" "$DATA" "$EXPECTED_TIME" "$NUM_OF_PROCESSES_PER_WORKER" "$EXPECTED_NUM_OF_BLOCKS"

echo "Time: $(date)"
echo "Duration: $(($(date +%s) - $(date -d "$CURRENT_TIME" +%s))) seconds"
