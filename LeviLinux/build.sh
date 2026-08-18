#!/bin/bash

set -e

PROJECT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="${PROJECT_DIR}/build"

mkdir -p "${BUILD_DIR}"

cd "${BUILD_DIR}"

# Configure CMake
cmake "${PROJECT_DIR}" -DCMAKE_BUILD_TYPE=Debug \
    -DCMAKE_INSTALL_PREFIX=/usr/local \
    -DCMAKE_CXX_COMPILER=g++ \
    -DCMAKE_C_COMPILER=gcc

# Build
cmake --build . --config Debug --parallel $(nproc)

# Run test executable
if [[ -f "test_core_engine" ]]; then
    echo "Running test_core_engine..."
    ./test_core_engine
fi

# Install (optional)
# cmake --install . --config Debug

echo "Build completed successfully!"
