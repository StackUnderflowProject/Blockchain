#!/bin/bash

# Assign arguments to variables
PROGRAM_PATH="/home/mic/CLionProjects/blockchain/cmake-build-debug/blockchain"
NUM_THREADS=4
DIFFICULTY=2
INPUT_FILE="/home/mic/CLionProjects/blockchain/input.txt"
EXPECTED_TIME=10

# Check if the input file exists
if [ ! -f "$INPUT_FILE" ]; then
  echo "Error: Input file '$INPUT_FILE' not found."
  exit 1
fi

# Read the contents of the input file
DATA=$(cat "$INPUT_FILE")

# Run the blockchain program with the file contents as the data argument
"$PROGRAM_PATH" "$NUM_THREADS" "$DIFFICULTY" "$DATA" "$EXPECTED_TIME"
