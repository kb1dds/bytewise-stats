/* Collect bytewise statistics of streams */

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
