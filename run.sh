#!/usr/bin/env bash

cd "$(dirname "$0")"
pwd

./build/serv >/dev/null 2>&1 & sleep 1; ./build/client localhost 8080
