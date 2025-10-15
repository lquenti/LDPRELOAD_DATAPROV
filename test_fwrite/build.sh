#!/bin/bash

set -e
gcc -fPIC -shared -o logger.so logger.c -ldl
gcc -o main main.c
LD_PRELOAD=./logger.so ./main
rm -f logger.so main

