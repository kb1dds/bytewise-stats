/* Collect bytewise statistics for a sliding window of bytes in a file
 *
 * Copyright (c) 2022, Michael Robinson
 */

#include <stdio.h>
#include <stdlib.h>
#include "bytewise_stats.h"

int main( int argc, char *argv[] ){
  int i, window_size, window_stride, window_offset;
  unsigned int counts[256];
  long offset;
  FILE *fp_data;

  if( argc != 5 ){
    printf("Usage: window_distribution window_size window_stride window_offset file\n");
    exit(-1);
  }

  sscanf(argv[1],"%d",&window_size);
  sscanf(argv[2],"%d",&window_stride);
  sscanf(argv[3],"%d",&window_offset);
  
  if( (fp_data=fopen(argv[4],"rt")) == NULL ){
    printf("Error opening input file: %s\n",argv[4]);
    exit(-2);
  }

  /* Column headers */
  printf("byte_offset");
  for( i = 0; i < 256; i ++ ){
    printf(",%d",i);
  }
  printf("\n");

  /* Consume data */
  offset = 0;
  while(window_size == bytewise_distribution( fp_data, window_size, window_stride, counts)){
    /* Store distribution in output file */
    printf("%ld",offset);
    for( i = 0; i < 256; i ++ ){
      printf(",%d",counts[i]);
    }
    printf("\n");

    fseek(fp_data,window_offset,SEEK_CUR);
    offset += window_offset;
  }
}
