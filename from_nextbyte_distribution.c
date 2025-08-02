/* Generate text from prefix and distribution
 *
 * Copyright (c) 2025, Michael Robinson
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bytewise_stats.h"

#define MAX_WINDOW_SIZE 1024

void get_byte_distribution( char *index_path, unsigned char *window, int window_size, unsigned int *counts );

int main( int argc, char *argv[] ){
  int i, j, k, count, counts[256], total_count, rv, window_size;
  unsigned char window[MAX_WINDOW_SIZE], byte;
  
  if( argc != 4 ){
    printf("Usage: from_nextbyte_distribution index_directory window_size count\n");
    exit(-1);
  }

  sscanf(argv[2],"%d",&window_size);
  sscanf(argv[3],"%d",&count);

  /* Grab prefix from stdin */
  scanf("%s",window);

  window[window_size] = '\0';

  for( j = 0; j < count; j ++ ){
    /* Get distribution for this window */
    printf("Window: %s\n", window);
    get_byte_distribution( argv[1], window, window_size, counts );

    /* Draw random character from this distribution */
    for( total_count = 0, i = 0; i < 256; i ++ )
      total_count += counts[i];
    rv = rand() * total_count/RAND_MAX;
    for( total_count = 0, byte = 255, i = 0; i < 256; i ++ ){
      total_count += counts[i];
      if( rv < total_count ){
	byte = i;
	break;
      }	
    }

    /* Send to stdout */
    printf("%c", byte);

    /* Advance window */
    for( i = 1; i < window_size+1; i ++){
      window[i-1]=window[i];
    }

    /* Tack the next byte onto end of window */
    window[window_size-1] = byte;
  }
}

void get_byte_distribution( char *index_path, unsigned char *window, int window_size, unsigned int *counts ){
  char index_file[MAX_WINDOW_SIZE*3];
  unsigned char *window_ptr;
  FILE *ifp;
  int cws, i;

  for( window_ptr = window, cws = window_size; cws > 2 ; window_ptr ++, cws -- ){
    /* Construct index filename */
    index_filename( index_file, index_path, window_ptr, cws );

    printf("Trying index file : %s ... ",index_file);

    if( (ifp = fopen(index_file, "rb")) != NULL){
      printf("found!\n");
      /* Index file found; pull counts and exit */
      fread(counts, (sizeof counts), 256, ifp);
      fclose(ifp);
      return;
    }
    printf("not found\n");
  }

  /* Default is uniform distribution */
  for( i = 0; i < 256; i ++ ){
    counts[i] = 1;
  }

}
