/* Collect bytewise statistics of streams
 *
 * Copyright (c) 2022,2025 Michael Robinson
 */

#include <stdio.h>
#include "bytewise_stats.h"

/* Collect counts of bytes from a file 
 * Input: fp = file pointer to read from
 *        window_size = number of bytes to read.  Negative means consume entire file
 *        stride = bytes to skip between reads
 * Output: fp = file pointer is returned to its original starting point
 *         counts = array of length 256 of counts of each byte
 */
int bytewise_distribution( FILE *fp, int window_size, int stride, unsigned int counts[] ) {
  int i;
  long initial_position, bytes_read;
  unsigned char byte;

  /* Initialize counts */
  for( i = 0; i < 256; i ++ ){
    counts[i] = 0;
  }

  /* We will return the file pointer to its original state after done */
  initial_position = ftell(fp);
  bytes_read = 0;

  /* Main read loop */
  while( window_size < 0 || bytes_read < window_size ){
    /* Consume byte */
    if(fread(&byte, 1, 1, fp)<1)
      break;

    /* Update count */
    counts[byte] ++;
    bytes_read ++;

    /* Stride to next location*/
    if( stride>1 ){
      if(!fseek(fp, stride-1, SEEK_CUR)){
	break;
      }
    }
  }

  /* Return file to its original position*/
  fseek(fp, initial_position, SEEK_SET);
  return(bytes_read);
}

/* Construct index filename from window */
void index_filename( char *index_file, char *index_path, unsigned char *window, int window_size ){
  int i, j;
  
  for( j = 0; index_path[j] != '\0' && j < 1024; j ++ ){
    index_file[j] = index_path[j];
  }
  index_file[j] = '/';
  for( i = 0, j ++; i < window_size; j += 2, i ++ ){
    sprintf( &index_file[j], "%x", (unsigned) window[i]);
  }
  j++;
  index_file[j] = 0;
}

/* Accumuate byte counts into a directory with count files organized by prefix
 */
int byte_prefixed_distribution( FILE *fp, char *index_path, int window_size ){
  FILE *ifp;
  long initial_position, bytes_read;
  char index_file[1024];
  unsigned int count;
  unsigned char byte, window[1024];
  int i, j, cws, k;

  /* Prefill window */
  for( i = 0; i < window_size; i ++ ){
    if(fread(&byte, 1, 1, fp)<1)
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
    for( cws = 2; cws < window_size; cws ++ ){
      /* Build index filename */
#ifdef DEBUG
      printf("Current window size %d\n",cws);
#endif
      index_filename( index_file, index_path, window, cws );

#ifdef DEBUG
      printf("Index file is : %s; next character is %x\n", index_file, (unsigned) window[cws]);
#endif

      /* Open index file */
      if( (ifp = fopen(index_file, "rb+")) == NULL){
	if( (ifp = fopen(index_file, "wb")) == NULL )
	  return -1;

#ifdef DEBUG
	printf("New index file\n");
#endif
	/* If index file does not exist, set all counts to zero except the next byte, which is 1 */
	for( k = 0; k < 256; k ++ ){
	  if( k == window[cws] ){
	    count = 1;
#ifdef DEBUG
	    printf("Character %x now has count 1\n", k);
#endif
	  }
	  else
	    count = 0;
	  
	  fwrite( &count, (sizeof count), 1, ifp );
	}
      }
      else{
	/* Increment count in index file */
	fseek( ifp, (sizeof count)*window[cws], SEEK_SET );
	fread( &count, (sizeof count), 1, ifp );
	fseek( ifp, (sizeof count)*window[cws], SEEK_SET );
	count ++;
	fwrite( &count, (sizeof count), 1, ifp );
      }

      /* Close index file */
      fclose(ifp);
    }

    /* Consume next byte */
    if( fread(&byte, 1, 1, fp) != 1 )
      break; /* No more data! */

    /* Advance window */
    for( i = 1; i < window_size; i ++){
      window[i-1]=window[i];
    }

    /* Tack the next byte onto end of window */
    window[window_size-1] = byte;
  }
  return 1;
}
  
