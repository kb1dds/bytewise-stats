bytewise_stats:
	gcc -c bytewise_stats.c

window_distribution: bytewise_stats
	gcc bytewise_stats.o window_distribution.c -lm -o window_distribution

build_distributions: bytewise_stats
	gcc bytewise_stats.o build_distributions.c -lm -o build_distributions

aggregate_nextbyte_distribution: bytewise_stats
	gcc bytewise_stats.o aggregate_nextbyte_distribution.c -lm -o aggregate_nextbyte_distribution

from_nextbyte_distribution: bytewise_stats
	gcc bytewise_stats.o from_nextbyte_distribution.c -lm -o from_nextbyte_distribution

byte_distribution_stats: bytewise_stats
	gcc bytewise_stats.o byte_distribution_stats.c -lm -o byte_distribution_stats

compare_byte_distributions: bytewise_stats
	gcc bytewise_stats.o compare_byte_distributions.c -lm -o compare_byte_distributions

colorby_nextbyte_distribution: bytewise_stats
	gcc bytewise_stats.o colorby_nextbyte_distribution.c -lm -o colorby_nextbyte_distribution

spaceby_nextbyte_distribution: bytewise_stats
	gcc bytewise_stats.o spaceby_nextbyte_distribution.c -lm -o spaceby_nextbyte_distribution

all: window_distribution build_distributions aggregate_nextbyte_distribution from_nextbyte_distribution compare_byte_distributions colorby_nextbyte_distribution byte_distribution_stats spaceby_nextbyte_distribution
