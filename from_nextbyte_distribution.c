/* Generate text from prefix and distribution
 *
 * Copyright (c) 2025, Michael Robinson
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "bytewise_stats.h"

#define MAX_WINDOW_SIZE 1024

unsigned int get_byte_distribution( char *index_path, unsigned char *window, int window_size, unsigned int *counts );

int main( int argc, char *argv[] ){
  int i, j, k;
  unsigned int count, counts[256], total_count, rv, window_size, cws;
  unsigned char window[MAX_WINDOW_SIZE], byte;

  /* Seed for random */
  srand(time(NULL));
  
  if( argc != 4 ){
    fprintf(stderr,"Usage: from_nextbyte_distribution index_directory window_size count\n");
    exit(-1);
  }

  sscanf(argv[2],"%d",&window_size);
  sscanf(argv[3],"%d",&count);

  /* Grab prefix from stdin */
  scanf("%s",window);

  window[window_size] = '\0';

  for( j = 0; j < count; j ++ ){
    /* Get distribution for this window */
#ifdef DEBUG
    fprintf(stderr,"Window: %s\n", window);
#endif
    cws = get_byte_distribution( argv[1], window, window_size, counts );

    /* Draw random character from this distribution */
    for( total_count = 0, i = 0; i < 256; i ++ ){
      total_count += counts[i];
    }
    rv = (unsigned int)( (double)rand() * (double) total_count / (double) RAND_MAX);
    for( total_count = 0, byte = 255, i = 0; i < 256; i ++ ){
      total_count += counts[i];
      if( rv < total_count ){
	byte = i;
	break;
      }	
    }

#ifdef DEBUG
    fprintf(stderr,"Random: %u %u %d -> %x:%c\n", rv, total_count, RAND_MAX, byte, byte);
#endif

    /* Send to stdout */
    printf("%c", byte);

    /* Advance window */
    for( i = window_size-cws; i < window_size; i ++){
      window[i-1]=window[i];
    }

    /* Tack the next byte onto end of window */
    window[window_size-1] = byte;
    window[window_size] = '\0';
  }
}

unsigned int get_byte_distribution( char *index_path, unsigned char *window, int window_size, unsigned int *counts ){
  char index_file[MAX_WINDOW_SIZE*3];
  unsigned char *window_ptr;
  FILE *ifp;
  int cws, i;

  for( window_ptr = window, cws = window_size; cws > 2 ; window_ptr ++, cws -- ){
    /* Construct index filename */
    index_filename( index_file, index_path, window_ptr, cws );

#ifdef DEBUG
    fprintf(stderr,"Trying index file (cws=%d) : %s ... ",cws,index_file);
#endif
    
    if( (ifp = fopen(index_file, "rb")) != NULL){
      /* Index file found; pull counts and exit */
#ifdef DEBUG
      fprintf(stderr,"found!\nRead %lu\n",fread(counts, (sizeof counts[0]), 256, ifp));
#else
      fread(counts, (sizeof counts[0]), 256, ifp);
#endif
	
      fclose(ifp);

#ifdef DEBUG
      for( i = 0; i < 256; i ++ )
	fprintf(stderr,"%x:%c:%u ",i,i,counts[i]);
      fprintf(stderr,"\n");
#endif
      return cws;
    }
#ifdef DEBUG
    fprintf(stderr,"not found\n");
#endif    
  }

  /* Default is uniform distribution */
  for( i = 0; i < 256; i ++ ){
    counts[i] = 1;
  }
  return 1;
}
