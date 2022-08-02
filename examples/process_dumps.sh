#!/bin/bash

for fl in `ls *.dump` ; 
  do ./window_distribution 2048 1 128 $fl > $fl.csv; 
done
