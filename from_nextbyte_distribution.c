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
  unsigned int count, counts[256], counts_temp[256], total_count, current_count, rv, window_size, cws;
  unsigned char window[MAX_WINDOW_SIZE], byte, current_byte;
  double entropy, default_entropy, current_entropy, temperature;
  int seed = time(NULL) ^ getpid();

  /* Seed for random */
  srand(seed);
  
  if( (argc != 4) && (argc != 5) ){
    fprintf(stderr,"Usage: from_nextbyte_distribution index_directory window_size count [temperature]\n");
    exit(-1);
  }

  sscanf(argv[2],"%d",&allow_varying_window);
  sscanf(argv[3],"%d",&count);

  if( argc == 5 )
    sscanf(argv[4],"%lf",&temperature);
  else
    temperature = 0.;

  /* If window_size as passed in is negative, then caller wants a specific window size only */
  if( allow_varying_window < 0 ){
    window_size = -allow_varying_window;
    allow_varying_window = 0;
  }
  else{
    window_size = allow_varying_window;
    allow_varying_window = 1;
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

    /* Variable window size empirical Bayesian estimation */
    if( temperature > 0. && allow_varying_window ){
      /* Initialize distribution */
      for( k = 0; k < 256; k ++ )
	counts[k] = 0;

      /* Load distributions from shorter windows */
      for( cws = 3; cws <= window_size; cws ++ ){
	get_byte_distribution( argv[1], window + (window_size-cws), cws, counts_temp, &fallback );

	/* Accumulate with weights */
	for( k = 0; k < 256; k ++ ){
	  if( counts_temp[k] )
	    counts[k] += (int)(pow(temperature,window_size-cws) * (1-temperature) * window_size * counts_temp[k]);
	}
      }
    }
    else
      get_byte_distribution( argv[1], window, window_size, counts, &fallback );
    
    /* Draw random character from this distribution */
    byte = draw_random_byte( counts, &total_count, &entropy );
    
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
