#!/usr/bin/env bash
set -e
SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
INJECTOR="$SCRIPT_DIR/libinjector.so"
gcc -shared -fPIC injector.c -o libinjector.so
gcc main.c -o main
bash -c 'sleep 5; echo "data sent via socket! Just press any key, its done" | socat - UNIX-CONNECT:/tmp/premain_injector_sock' &
LD_PRELOAD=$INJECTOR "$SCRIPT_DIR/main"
