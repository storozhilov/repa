#!/usr/bin/env bash

set -e

echo "Building sr"

set -x
g++ src/sr.cpp -o sr
