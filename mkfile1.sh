#!/bin/bash

if [ $# -ne 2 ]; then
    echo "Usage: $0 <buffer_size> <n>"
    exit 1
fi

buffer_size=$1
iterations=$2

g++ main1.cpp -o main1 -pthread
./main1 $buffer_size $iterations