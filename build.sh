#!/usr/bin/env bash

cd "$(dirname "$0")"

# TODO: different modes

# abort on error
set -o errexit

# TODO: add shell lib
cargo build
mkdir -p build/

cp target/debug/server build/
cp target/debug/client build/
