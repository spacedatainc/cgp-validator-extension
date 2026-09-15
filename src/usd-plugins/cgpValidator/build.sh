#!/bin/sh

export USD_ROOT=/ws/rkat/usd-build/usd2608

cmake \
    -B build \
    -DCMAKE_INSTALL_PREFIX=install \
    -DCMAKE_BUILD_TYPE=Release \
    -DUSD_ROOT=${USD_ROOT} \
    .

if [[ -d "build" ]]; then
    echo "Build directory exists. Cleaning up..."
    rm -rf build
fi
cmake --build build
cmake --install build