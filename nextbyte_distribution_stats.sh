#!/bin/bash
#
# Generate potentially useful stats about next-byte distributions
#
# Copyright (c) 2025, Michael Robinson

echo "prefix,total_count,entropy,max_byte,max_count,nz"

for fl in `ls $1`;
do
    echo -n "$fl,"
    ./byte_distribution_stats $1/$fl
done
