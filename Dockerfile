FROM ubuntu:latest
LABEL authors="mic"

# Install required packages
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    libssl-dev \
    openmpi-bin \
    libopenmpi-dev \
    && rm -rf /var/lib/apt/lists/*

# Set the working directory
WORKDIR /app

# Copy the source code and CMakeLists.txt into the container
COPY . .

# Build the project
RUN mkdir build && cd build && \
    cmake -DCMAKE_BUILD_TYPE=Release .. && \
    make

# Set the entry point for running the program
ENV OMPI_ALLOW_RUN_AS_ROOT=1
ENV OMPI_ALLOW_RUN_AS_ROOT_CONFIRM=1

ENTRYPOINT ["mpirun"]
CMD ["--allow-run-as-root", "-np", "4", "./build/blockchain", "3", "sampledata", "5", "2", "10"]