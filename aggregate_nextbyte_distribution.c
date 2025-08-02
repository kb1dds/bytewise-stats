/* Collect bytewise statistics of a file
 *
 * Copyright (c) 2025, Michael Robinson
 */

#include <stdio.h>
#include <stdlib.h>
#include "bytewise_stats.h"

int main( int argc, char *argv[] ){
  int i, window_size;
  unsigned int counts[256];
  char line[1024], *file, *tag;
  FILE *fp_in, *fp_data;

  if( argc != 4 ){
    printf("Usage: aggregate_nextbyte_distribution file index_directory window_size\n");
    exit(-1);
  }

  sscanf(argv[3],"%d",&window_size);
        
  if( (fp_data=fopen(argv[1],"rb")) != NULL ){
    /* Collect distribution of bytes */
    byte_prefixed_distribution( fp_data, argv[2], window_size );
    
    fclose(fp_data);
  }
  else{
    printf("Error opening data file: %s\n",file);
  }
}
