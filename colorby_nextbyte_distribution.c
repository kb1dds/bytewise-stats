/* Color text based on prefix probability
 *
 * Copyright (c) 2025, Michael Robinson
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "bytewise_stats.h"

/* Compile-time flag: default is to color by probability of the specific byte.
 * Define TAILCOLOR to instead color by the probability of the tail of the distribution,
 * which is a weaker test (higher p values)
 */

int main( int argc, char *argv[] ){
  int i, fallback;
  unsigned int counts[256], window_size;
  unsigned char window[MAX_WINDOW_SIZE], byte;
  double p;
  
  if( argc != 3 ){
    fprintf(stderr,"Usage: coloby_nextbyte_distribution index_directory window_size\n");
    exit(-1);
  }

  sscanf(argv[2],"%d",&window_size);

  /* Prefill window */
  for( i = 0; i < window_size; i ++ ){
    if(fread(&byte, 1, 1, stdin)<1)
      return -1; /* Didn't manage to get a full window */
    window[i] = byte;
  }

  /* Main read loop */
  while(1) {
#ifdef DEBUG
    printf("Window is: ");
    for( i = 0; i< window_size; i ++ ){
      printf("%c",window[i]);
    }
    printf("\n");
#endif

    /* Load byte histogram for the prefix of the window */ 
    get_byte_distribution( argv[1], window, window_size-1, counts, &fallback );

    /* Estimate probability of the next byte being what it is */
#ifdef TAILCOLOR
    p = onebyte_pval( counts, byte );
#else
    for( i = 0, p = 0.; i < 256; i ++ ){
      p += (double)counts[i];
    }
    p = counts[byte] / p;
#endif
    
    /* Colorize and send to stdout */
    i = (int)(255.0*(p));
    if(fallback){
      printf("\e[38;2;%d;0;0m",255-i);
    }
    else{
      printf("\e[38;2;%d;%d;%dm",i,i,i);
    }
    printf("%c", byte);
    if(fallback)
      printf("\e[0m");

    /* Consume next byte */
    if( fread(&byte, 1, 1, stdin) != 1 )
      break; /* No more data! */

    /* Advance window */
    for( i = 1; i < window_size; i ++){
      window[i-1]=window[i];
    }

    /* Tack the next byte onto end of window */
    window[window_size-1] = byte;
  }
}
