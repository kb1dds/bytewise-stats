/* Collect bytewise statistics of a list of files
 *
 * Copyright (c) 2022, Michael Robinson
 */

#include <stdio.h>
#include "bytewise_stats.h"

int main( int argc, char *argv ){
  int i;
  unsigned int counts[256];
  char line[1024], file[256], tag[256];
  FILE *fp_in, *fp_out, *fp_data;

  if( argc != 3 ){
    printf("Usage: build_distributions list.csv output.csv\n");
    exit(-1);
  }
  
  if( (fp_in=fopen(argv[1],"rt")) == NULL ){
    printf("Error opening input file: %s\n",argv[1]);
    exit(-2);
  }

  if( (fp_out=fopen(argv[2],"wt")) == NULL ){
    printf("Error opening output file: %s\n",argv[2]);
    exit(-2);
  }

  /* First column of output is the byte value */
  fprintf(fp_out,"Byte");
  for( i = 0; i < 256; i ++ ){
    fprintf(fp_out,",%d",i);
  }
  fprintf(fp_out,"\n");

  /* Consume header and begin reading */
  if( fgets(line,1024,fp_in) != NULL ){
    
    while(fgets(line,1024,fp_in) != NULL){
      sscanf(line,"%s,%s\n",tag,file);

      if( (fp_data=fopen(file,"rb")) != NULL ){
	/* Collect distribution of bytes */
	bytewise_distribution( fp_data, -1, 1, counts );

	/* Store distribution in output file */
	fprintf(fp_out,"%s",tag);
	for( i = 0; i < 256; i ++ ){
	  fprintf(fp_out,",%d",counts[i]);
	}
	fprintf(fp_out,"\n");
	fclose(fp_data);
      }
      else{
	printf("Error opening data file: %s\n",file);
      }
    }
  }
  
  fclose(fp_in);
  fclose(fp_out);
}
