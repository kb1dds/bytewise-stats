#!/bin/bash
#
# Generate comparison report for two next-byte distributions
#
# Copyright (c) 2025, Michael Robinson

echo "prefix,reference_count,test_count,chisq,pvalue"

for fl in `ls $2`;
do
    echo -n "$fl,"
    ./compare_byte_distributions $1/$fl $2/$fl
done
