/* Collect bytewise statistics of a list of files
 *
 * Copyright (c) 2022, Michael Robinson
 */

#include <stdio.h>
#include <stdlib.h>
#include "bytewise_stats.h"

int main( int argc, char *argv[] ){
  int i;
  unsigned int counts[256];
  char line[1024], *file, *tag;
  FILE *fp_in, *fp_data;

  if( argc != 2 ){
    printf("Usage: build_distributions list.csv\n");
    exit(-1);
  }
  
  if( (fp_in=fopen(argv[1],"rt")) == NULL ){
    printf("Error opening input file: %s\n",argv[1]);
    exit(-2);
  }

  /* Column Headers are the byte values */
  printf("file_tag");
  for( i = 0; i < 256; i ++ ){
    printf(",%d",i);
  }
  printf("\n");

  /* Consume header and begin reading */
  if( fgets(line,1024,fp_in) != NULL ){
    
    while(fgets(line,1024,fp_in) != NULL){
      /* Parse tag and filename from line */
      tag = line;
      for( i = 0; i < 1024; i ++ ){
	if(line[i] == ','){
	  line[i] = 0;
	  file=&line[i+1];
	}
	if(line[i] == '\n'){
	  line[i] = 0;
	}
      }
      
      if( (fp_data=fopen(file,"rb")) != NULL ){
	/* Collect distribution of bytes */
	bytewise_distribution( fp_data, -1, 1, counts );

	/* Store distribution in output file */
	printf("%s",tag);
	for( i = 0; i < 256; i ++ ){
	  printf(",%d",counts[i]);
	}
	printf("\n");
	fclose(fp_data);
      }
      else{
	printf("Error opening data file: %s\n",file);
      }
    }
  }
  
  fclose(fp_in);
}
