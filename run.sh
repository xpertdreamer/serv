#!/usr/bin/env bash

cd "$(dirname "$0")"

./build/server >/dev/null 2>&1 & sleep 1; ./build/client 127.0.0.1 4321
