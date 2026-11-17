#!/usr/bin/env bash

cd "$(dirname "$0")"

export LD_LIBRARY_PATH="$LD_LIBRARY_PATH:$(pwd)/build"

./build/serv >/dev/null 2>&1 & sleep 1; ./build/client localhost 8080
