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
  int i;
  int reference_total, test_total;

  if( argc != 3 ){
    fprintf(stderr,"Usage: compare_byte_distributions reference_file test_file\n");
    exit(-1);
  }

  if( ( fp = fopen(argv[1], "rb") ) == NULL ){
    reference_total = -1;
  }
  else{
    reference_total = 0;
    fread(reference_counts,(sizeof reference_counts),256,fp);
    fclose(fp);
  }
  
  if( ( fp = fopen(argv[2], "rb") ) == NULL ){
    test_total = -1;
  }
  else{
    test_total = 0;
    fread(test_counts,(sizeof test_counts),256,fp);
    fclose(fp);
  }
  
  for( i = 0; i < 256; i ++ ){
    if(reference_total >= 0 )
      reference_total += reference_counts[i];

    if( test_total >= 0 )
      test_total += test_counts[i];
  }

  if( reference_total >= 0 && test_total >= 0 ){
    p = byte_distribution_compare( reference_counts, test_counts, &chisq );
  }
  else{
    chisq = 1e100;
    p = 0;
  }
  
  printf("%d,%d,%g,%g\n",reference_total,test_total,chisq,p);
}
