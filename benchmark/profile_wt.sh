#!/bin/bash

# Experimental profiling script

set -e

TIMESTAMP=$(date +"%Y-%m-%d--%H-%M-%S")

valgrind \
    --tool=callgrind \
    --dump-instr=yes \
    --simulate-cache=yes \
    --collect-jumps=yes \
    --callgrind-out-file=profile/temp/p-${TIMESTAMP}.out \
    ${BIN_DIR}/benchmark_wt \
        --size 32MiB \
        --iterations 3 \
        --alphabet_size 16384 \
        --implementation basic \
        --implementation twophase_4_4 \
        --implementation twophase_4_8 \
        --implementation twophase_8_4 \
        --implementation twophase_8_8 \
        --implementation twophase_pext

