/* Compare two byte histograms using Chi^2 test
 *
 * Copyright (c) 2025, Michael Robinson
 */

#include <stdio.h>
#include <stdlib.h>
#include "bytewise_stats.h"

int main( int argc, char *argv[] ){
  FILE *fp;
  unsigned int reference_counts[256], test_counts[256];
  double chisq, p;

  if( argc != 3 ){
    fprintf(stderr,"Usage: compare_byte_distributions reference_file test_file\n");
    exit(-1);
  }

  if( ( fp = fopen(argv[1], "rb") ) == NULL ){
    fprintf(stderr,"Error opening: %s\n",argv[1]);
    exit(-1);
  }
  fread(reference_counts,(sizeof reference_counts),256,fp);
  fclose(fp);

  if( ( fp = fopen(argv[2], "rb") ) == NULL ){
    fprintf(stderr,"Error opening: %s\n",argv[2]);
    exit(-1);
  }
  fread(test_counts,(sizeof test_counts),256,fp);
  fclose(fp);

  p = byte_distribution_compare( reference_counts, test_counts, &chisq );

  printf("%g,%g\n",chisq,p);
}
