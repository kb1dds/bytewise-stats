/* Generate text from prefix and distribution
 *
 * Copyright (c) 2025, Michael Robinson
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <unistd.h>
#include "bytewise_stats.h"

unsigned char draw_random_byte( unsigned int *counts, unsigned int *tc, double *entropy );

int main( int argc, char *argv[] ){
  int i, j, k, fallback, allow_varying_window;
  unsigned int count, counts[256], total_count, current_count, rv, window_size, cws;
  unsigned char window[MAX_WINDOW_SIZE], byte, current_byte;
  double entropy, default_entropy, current_entropy;
  int seed = time(NULL) ^ getpid();

  /* Seed for random */
  srand(seed);
  
  if( argc != 4 ){
    fprintf(stderr,"Usage: from_nextbyte_distribution index_directory window_size count\n");
    exit(-1);
  }

  sscanf(argv[2],"%d",&allow_varying_window);
  sscanf(argv[3],"%d",&count);

  /* If window_size as passed in is negative, then caller permits window size to vary randomly */
  if( allow_varying_window < 0 ){
    window_size = -allow_varying_window;
    allow_varying_window = 1;
  }
  else{
    window_size = allow_varying_window;
    allow_varying_window = 0;
  }

  /* Entropy calibration */
  get_byte_distribution( argv[1], NULL, window_size, counts, NULL );
  draw_random_byte(counts, NULL, &default_entropy);

  /* Grab prefix from stdin */
  scanf("%s",window);

  total_count = strlen( window );
#ifdef DEBUG
  fprintf(stderr,"Characters supplied: %d\n",total_count);
#endif

  /* If prefix isn't long enough, prepend with random characters using the global histogram */
  if( total_count < window_size ){
    current_count = window_size - total_count;
    
    for( i = window_size-1; i >= current_count; i -- ){
      window[i] = window[i-current_count];	    
    }
    get_byte_distribution( argv[1], NULL, window_size, counts, NULL );
    for( i = current_count-1; i >= 0; i --) {
      window[i] = draw_random_byte(counts, NULL, NULL);
    }
  }
  window[window_size] = '\0';

  printf("%s", window);

  for( j = 0; j < count; j ++ ){
    if( allow_varying_window ){
      /* Variable window size logic */
      for( cws = window_size, entropy = -1; cws >= 2; cws -- ){
	/* Get distribution for this window */
	get_byte_distribution( argv[1], window, cws, counts, &fallback );

	/* Draw random character from this distribution */
	current_byte = draw_random_byte( counts, &total_count, &current_entropy );

	/* Select the lowest entropy window */
	if( entropy < 0 || current_entropy < entropy ){
	  byte = current_byte;
	  entropy = current_entropy;
	}
      }
    }
    else{
      /* Fixed window size */
      cws = window_size;

#ifdef DEBUG
      fprintf(stderr,"Window (size = %d) : %s\n", cws, window);
#endif

      /* Get distribution for this window */
      get_byte_distribution( argv[1], window, cws, counts, &fallback );
    
      /* Draw random character from this distribution */
      byte = draw_random_byte( counts, &total_count, &entropy );
    }
    
    /* Send to stdout */
#ifdef ANSI_COLOR
    i = (int)(255*(default_entropy - entropy)/default_entropy);
    i = (i<100)?100:i;
    if(fallback){
      printf("\e[38;2;%d;0;0m",255-i);
    }
    else{
      printf("\e[38;2;%d;%d;%dm",i,i,i);
    }
#endif
    printf("%c", byte);
#ifdef ANSI_COLOR
    if(fallback)
      printf("\e[0m");
#endif

    /* Advance window */
    for( i = 1; i < window_size; i ++){
      window[i-1]=window[i];
    }

    /* Tack the next byte onto end of window */
    window[window_size-1] = byte;
    window[window_size] = '\0';
  }
}

/* Draw random character from this distribution
 *
 * Input: counts = byte histogram to draw from (array of 256)
 * Output: tc = total count in histogram
 *         entropy = entropy of histogram
 * Returns: the random byte selected
 */
unsigned char draw_random_byte( unsigned int *counts, unsigned int *tc, double *entropy ){
  unsigned int total_count, current_count, i, rv;
  unsigned char byte;
  double H;

  for( total_count = 0, i = 0; i < 256; i ++ ){
    total_count += counts[i];
  }
  if( entropy != NULL ){
    for( H = 0., i = 0; i < 256; i ++ ){
      if(counts[i] > 0)
	H -= counts[i] * log10((double)counts[i]/(double)total_count)/log10(2.0);
    }
    H /= total_count;
    *entropy = H;
  }
  
  rv = 1+(unsigned int)( (double)rand() * (double) total_count / (double) RAND_MAX);
  for( current_count = 0, byte = 255, i = 0; i < 256; i ++ ){
    current_count += counts[i];
    if( rv <= current_count ){
      byte = i;
      break;
    }	
  }
  if( tc != NULL )
    *tc = total_count;

#ifdef DEBUG
  fprintf(stderr,"Random: %u %u %d -> %x:%c\n", rv, total_count, RAND_MAX, byte, byte);
#endif
  
  return byte;
}
