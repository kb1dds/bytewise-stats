/* Generate potentially useful statistics for a byte histogram
 *
 * Copyright (c) 2025, Michael Robinson
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

int main( int argc, char *argv[] ){
  FILE *fp;
  unsigned int counts[256];
  double entropy;
  int i, max_byte, nz;
  unsigned int total_count, max_count;

  if( argc != 2 ){
    fprintf(stderr,"Usage: byte_distribution_stats histogram_file\n");
    exit(-1);
  }

  if( ( fp = fopen(argv[1], "rb") ) == NULL ){
    fprintf(stderr,"Error opening: %s\n",argv[1]);
    exit(-1);
  }
  fread(counts,(sizeof counts),256,fp);
  fclose(fp);

  for( total_count = 0, i = 0, max_byte = 0, max_count = 0, nz = 0; i < 256; i ++ ){
    total_count += counts[i];
    if( counts[i] > max_count ){
      max_count = counts[i];
      max_byte = i;
    }
    if( counts[i] == 0 )
      nz ++;
  }
  for( entropy = 0., i = 0; i < 256; i ++ ){
    if(counts[i] > 0)
      entropy -= counts[i] * log10((double)counts[i]/(double)total_count)/log10(2.0);
  }
  entropy /= total_count;

  printf("%d,%g,%02x,%d,%d\n",total_count,entropy,max_byte,max_count,nz);
}
