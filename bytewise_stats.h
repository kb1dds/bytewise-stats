/* Collect bytewise statistics of streams
 *
 * Copyright (c) 2022, 2025, Michael Robinson
 */

#ifndef _BYTEWISE_STATS_H_
#define _BYTEWISE_STATS_H_
int bytewise_distribution( FILE *fp, int window_size, int stride, unsigned int counts[] );
int byte_prefixed_distribution( FILE *fp, char *index_path, int window_size );
void index_filename( char *index_file, char *index_path, unsigned char *window, int window_size );
double bytewise_distribution_compare( unsigned int reference_counts[], unsigned int observed_counts[], double *chisq_out );
double chisquared_pval(double chisq, double dof);

#endif /*_BYTEWISE_STATS_H_*/
