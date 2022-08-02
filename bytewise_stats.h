/* Collect bytewise statistics of streams
 *
 * Copyright (c) 2022, Michael Robinson
 */

#ifndef _BYTEWISE_STATS_H_
#define _BYTEWISE_STATS_H_
int bytewise_distribution( FILE *fp, int window_size, int stride, unsigned int counts[] );

#endif /*_BYTEWISE_STATS_H_*/
