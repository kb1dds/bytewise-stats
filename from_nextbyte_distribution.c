/* Generate text from prefix and distribution
 *
 * Copyright (c) 2025, Michael Robinson
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include "bytewise_stats.h"

#define MAX_WINDOW_SIZE 1024

unsigned char random_draw( unsigned int *counts, unsigned int *tc, double *entropy );

unsigned int get_byte_distribution( char *index_path, unsigned char *window, int window_size, unsigned int *counts, int *fallback );

int main( int argc, char *argv[] ){
  int i, j, k, fallback;
  unsigned int count, counts[256], total_count, current_count, rv, window_size, cws;
  unsigned char window[MAX_WINDOW_SIZE], byte;
  double entropy, default_entropy;

  /* Seed for random */
  srand(time(NULL));
  
  if( argc != 4 ){
    fprintf(stderr,"Usage: from_nextbyte_distribution index_directory window_size count\n");
    exit(-1);
  }

  sscanf(argv[2],"%d",&window_size);
  sscanf(argv[3],"%d",&count);

  /* Entropy calibration */
  get_byte_distribution( argv[1], NULL, window_size, counts, NULL );
  random_draw(counts, NULL, &default_entropy);

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
      window[i] = random_draw(counts, NULL, NULL);
    }
  }
  window[window_size] = '\0';

  printf("%s", window);

  for( j = 0; j < count; j ++ ){
    /* Get distribution for this window */
#ifdef DEBUG
    fprintf(stderr,"Window: %s\n", window);
#endif
    cws = get_byte_distribution( argv[1], window, window_size, counts, &fallback );
    
    /* Draw random character from this distribution */
    byte = random_draw( counts, &total_count, &entropy );

#ifdef DEBUG
    fprintf(stderr,"Random: %u %u %d -> %x:%c\n", rv, total_count, RAND_MAX, byte, byte);
#endif

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

/* Draw random character from this distribution */
unsigned char random_draw( unsigned int *counts, unsigned int *tc, double *entropy ){
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
  
  rv = (unsigned int)( (double)rand() * (double) total_count / (double) RAND_MAX);
  for( current_count = 0, byte = 255, i = 0; i < 256; i ++ ){
    current_count += counts[i];
    if( rv < current_count ){
      byte = i;
      break;
    }	
  }
  if( tc != NULL )
    *tc = total_count;
  
  return byte;
}


unsigned int get_byte_distribution( char *index_path, unsigned char *window, int window_size, unsigned int *counts, int *fallback ){
  char index_file[MAX_WINDOW_SIZE*3];
  unsigned char *window_ptr;
  FILE *ifp;
  int cws, i;

  if( window != NULL ){
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
	if( fallback != NULL )
	  *fallback = 0;
	return cws;
      }
#ifdef DEBUG
      fprintf(stderr,"not found\n");
#endif    
    }
  }

  /* Look for a global histogram */
  if( fallback != NULL )
    *fallback = 1;
  index_filename( index_file, index_path, window_ptr, 0 );

#ifdef DEBUG
  fprintf(stderr,"Falling back to global distribution: %s...",index_file);
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
  
  /* Default is uniform distribution */
  for( i = 0; i < 256; i ++ ){
    counts[i] = 1;
  }
  return 1;
}
