#!/bin/bash

if [ $# -ne 2 ]; then
    echo "Usage: $0 <buffer_size> <n>"
    exit 1
fi

buffer_size=$1
iterations=$2

g++ main2.cpp -o main2 -pthread
./main2 $buffer_size $iterations