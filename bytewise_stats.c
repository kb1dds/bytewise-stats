/* Collect bytewise statistics of streams
 *
 * Copyright (c) 2022,2025 Michael Robinson
 */

#include <stdio.h>
#include <math.h>
#include "bytewise_stats.h"

static double igf(double S, double Z);

/* Collect counts of bytes from a file 
 * Input: fp = file pointer to read from
 *        window_size = number of bytes to read.  Negative means consume entire file
 *        stride = bytes to skip between reads
 * Output: fp = file pointer is returned to its original starting point
 *         counts = array of length 256 of counts of each byte
 */
int byte_distribution( FILE *fp, int window_size, int stride, unsigned int counts[] ) {
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
  if( window_size > 0 ){
    for( i = 0, j ++; i < window_size; j += 2, i ++ ){
      sprintf( &index_file[j], "%02x", (unsigned) window[i]);
    }
    j++;
    index_file[j] = 0;
  }
  else{
    index_file[j+1] = 'n';
    index_file[j+2] = 'u';
    index_file[j+3] = 'l';
    index_file[j+4] = 'l';
    index_file[j+5] = 0;
  }
}

/* Accumuate byte counts into a directory with count files organized by prefix
 *
 * Input: fp = byte stream (reads until EOF encountered)
 *        index_path = Path to histogram files
 *        window_size = maximum size of window to use as prefix
 * Returns: 0 if success, -1 if file error
 */
int byte_prefixed_distribution( FILE *fp, char *index_path, int window_size ){
  FILE *ifp;
  long initial_position, bytes_read;
  char index_file[1024];
  unsigned int count, counts[256], oldcounts[256];
  unsigned char byte, window[1024];
  int i, j, cws, k;

  /* Present global histogram */
  for( i = 0; i < 256; i ++ )
    counts[i] = 0;

  /* Prefill window */
  for( i = 0; i < window_size; i ++ ){
    if(fread(&byte, 1, 1, fp)<1)
      return -1; /* Didn't manage to get a full window */
    window[i] = byte;
    counts[byte] ++;
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
	  return -1; /* Fatal error */

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

    /* Global tally */
    counts[byte] ++;

    /* Advance window */
    for( i = 1; i < window_size; i ++){
      window[i-1]=window[i];
    }

    /* Tack the next byte onto end of window */
    window[window_size-1] = byte;
  }

  /* Save global histogram */
  index_filename( index_file, index_path, window, 0 );
  if( (ifp = fopen(index_file, "rb+")) == NULL){
    if( (ifp = fopen(index_file, "wb")) == NULL )
      return -1; /* Fatal error */

    fwrite(counts,(sizeof count),256,ifp);
  }
  else{
    fread(oldcounts,(sizeof count),256,ifp);
    fseek( ifp, 0, SEEK_SET );
    for( i = 0; i < 256; i ++ ){
      counts[i] += oldcounts[i];
    }
    fwrite(counts,(sizeof count),256,ifp);
  }
  fclose(ifp);
  
  return 1;
}

/* Load conditional byte distribution given a window
 * 
 * Input: index_path = Path to histogram files
 *        window = contents of window
 *        window_size = length of window
 * Output counts = histogram of counts for byte distribution (array of 256)
 *	  fallback = 0: window was found, counts loaded from corresponding histogram
 *                   1: window not found, so counts was loaded from the global byte histogram
 *                   2: window not found, global histogram not found, so uniform distribution returned
 */
unsigned int get_byte_distribution( char *index_path, unsigned char *window, int window_size, unsigned int *counts, int *fallback ){
  char index_file[MAX_WINDOW_SIZE*3];
  unsigned char *window_ptr;
  FILE *ifp;
  int cws, i;

  if( window != NULL ){
    for( window_ptr = window, cws = window_size; cws >= 1 ; window_ptr ++, cws -- ){
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
  if( fallback != NULL )
    *fallback = 2;
    
  for( i = 0; i < 256; i ++ ){
    counts[i] = 1;
  }
  return 1;
}


/* Run Chi^2 goodness of fit test to compare two byte histograms
 * Input: expected_counts = histogram of counts for reference distribution (array of 256)
 *        observed_counts = histogram of counts for test distribution (array of 256)
 * Output: chisq_out = test statistic (or NULL if unwanted)
 * Returns: p-value for test Chi^2 goodness of fit distribution
 * Note: the reference distribution counts are floored to 1 internally to avoid issues
 */
double byte_distribution_compare( unsigned int reference_counts[], unsigned int observed_counts[], double *chisq_out ){
  int i;
  double total_reference, total_observed;
  double chisq, expected;

  /* Compute total counts for both histograms */
  for( i = 0, total_reference = 0, total_observed; i < 256; i ++ ){
    total_reference += (double) reference_counts[i];
    total_observed += (double) observed_counts[i];

    /* Reference counts are no less than 1 */
    if( reference_counts[i] == 0 )
      total_reference ++;
  }
  
  for( i = 0, chisq = 0.0; i < 256; i ++ ){
    /* Reference counts are no less than 1 */
    if( reference_counts[i] != 0 )
      expected = total_observed * (double)reference_counts[i] / total_reference;
    else
      expected = total_observed / total_reference;

    /* Accumulate chi^2 */
    chisq += (observed_counts[i]-expected)*(observed_counts[i]-expected)/expected;
  }

  if( chisq_out )
    *chisq_out = chisq;

  return chisquared_pval( chisq, 255 );
}

/* What's the probability that this byte or one less common was drawn from a given distribution?
 *
 * Input: reference_counts = histogram of counts for reference distribution (array of 256)
 *        byte             = the byte to test
 * Output: none
 * Returns: p-value for the test
 */
double onebyte_pval( unsigned int reference_counts[], unsigned char byte ){
  int i;
  double total_count; 
  double pvalue, byte_probability, ptemp;

  /* Total number of bytes in distribution */
  for( i = 0, total_count = 0.0; i < 256; i ++ )
    total_count += (double) reference_counts[i];

  /* This byte's probability */
  byte_probability = (double)reference_counts[byte];

  /* Aggregate tail of distribution */
  for( i = 0, pvalue = 0.; i < 256; i ++ ){
    ptemp = (double)reference_counts[i];
    if( i == byte || ptemp <= byte_probability )
      pvalue += ptemp;
  }

  return pvalue / total_count;
}

/* Estimate p-value for Chi^2 distribution
 * Based upon the code at
 *  [https://www.codeproject.com/Articles/432194/How-to-Calculate-the-Chi-Squared-P-Value]
 * See also [https://en.wikipedia.org/wiki/Chi-squared_distribution#Cumulative_distribution_function]
 * Input: chisq = test statistic
 *        dof   = degrees of freedom
 * Returns: p-value
 */
double chisquared_pval(double chisq, double dof){
  double K, X, pvalue;

  /* Bounds check */
  if(chisq < 0 || dof < 1)
    return 0.0;
    
  /* Special case if dof = 2 */
  X = chisq * 0.5;
  if(dof == 2)
    return exp(-1.0 * X);

  /* Otherwise use the incomplete gamma function for the numerator */
  K = dof * 0.5;
  pvalue = igf(K, X);

  /* Error handling */
  if(isnan(pvalue) || isinf(pvalue))
    return 1e-14;

  /* Gamma function in the denominator */
  pvalue /= tgamma(K);
  
  return (1.0 - pvalue);
}

/* Incomplete gamma function
 * Based upon the code at
 *  [https://www.codeproject.com/Articles/432194/How-to-Calculate-the-Chi-Squared-P-Value]
 * which evaluates the power series given in [https://en.wikipedia.org/wiki/Incomplete_gamma_function#Evaluation_formulae]
 */
static double igf(double S, double Z)
{
  double Sc, sum, num, denom;
   
  if(Z < 0.0)
    return 0.0;

  /* Leading coefficient */
  Sc = (1.0 / S);
  Sc *= pow(Z, S);
  Sc *= exp(-Z);

  /* Power series terms; run until they get small */
  for( sum = 1.0, num = 1.0, denom = 1.0; (num / denom) > 1e-8; ){
    num *= Z;
    S++;
    denom *= S;
    sum += (num / denom);
  }

  /* Final assembly */
  return sum * Sc;
}
