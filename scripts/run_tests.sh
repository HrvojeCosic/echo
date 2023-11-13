#!/bin/bash

# (Re)build first
./scripts/build.sh ON

# Find and run all test files
find build/test -type f -name "*_test" -exec {} \;
