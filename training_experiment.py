#!/bin/python
#
# Explore how much time and space training from a corpus varies with window size

import subprocess
import tqdm
import time

# Recommend making a temporary RAM disk for the working path:
#  $ sudo mount -t tmpfs -o size=16384G tmpfs ~/tmp/

corpus = 'corpus_1.txt'
max_window = 12
model_path = '~/tmp/' # Beware.  This directory is routinely deleted!

with open('results.csv','a') as fp:
    if fp.tell() == 0:
        fp.write('corpus,window_size,training_start_time,runtime,model_size\n')

    for window in tqdm.tqdm(range(2,max_window)):
        subprocess.call('find {} -type f -name "*" -delete'.format(model_path), shell=True)
        
        training_start = time.time()
        subprocess.call('cat {} | ./aggregate_nextbyte_distribution {} {}'.format(corpus,model_path,window), shell=True)
        training_stop = time.time()

        model_size = int(subprocess.check_output('du {} -s'.format(model_path), shell=True).split(b'\t')[0])

        fp.write('{},{},{},{},{}\n'.format(corpus,window,training_start,training_stop-training_start,model_size))
