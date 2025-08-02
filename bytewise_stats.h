/* Collect bytewise statistics of streams
 *
 * Copyright (c) 2022, 2025, Michael Robinson
 */

#ifndef _BYTEWISE_STATS_H_
#define _BYTEWISE_STATS_H_
int bytewise_distribution( FILE *fp, int window_size, int stride, unsigned int counts[] );
int byte_prefixed_distribution( FILE *fp, char *index_path, int window_size );

#endif /*_BYTEWISE_STATS_H_*/
