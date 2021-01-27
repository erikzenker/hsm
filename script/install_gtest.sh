#!/bin/bash

if [ -d "build/dependencies/gtest" ]; then
    exit 0
fi

set -eux

mkdir -p build/dependencies
cd build/dependencies

git clone https://github.com/google/googletest
(cd googletest; git checkout 6c58c11d5497b6ee1df3cb400ce30deb72fc28c0)

cmake -S googletest -B googletest/build \
    -D "CMAKE_INSTALL_PREFIX=${PWD}/gtest" \
    -D CMAKE_BUILD_TYPE=Release

cmake --build googletest/build

cmake --install googletest/build

rm -rf googletest
