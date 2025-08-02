/* Collect bytewise statistics of stdin
 *
 * Copyright (c) 2025, Michael Robinson
 */

#include <stdio.h>
#include <stdlib.h>
#include "bytewise_stats.h"

int main( int argc, char *argv[] ){
  int window_size;

  if( argc != 3 ){
    printf("Usage: aggregate_nextbyte_distribution index_directory window_size\n");
    exit(-1);
  }

  sscanf(argv[2],"%d",&window_size);
        
  byte_prefixed_distribution( stdin, argv[1], window_size );
}
