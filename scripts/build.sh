#!/bin/bash

WITH_TESTS=$1

if [ -z "$WITH_TESTS" ]; then
    WITH_TESTS="OFF"
fi

mkdir -p build
cd build
cmake .. -DWITH_TESTS=$WITH_TESTS
make