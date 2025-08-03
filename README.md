# bytewise-stats: Looking a statistics of byte usage in files

## Four utilities are provided:
* `window_distribution` : runs a sliding window over a file, producing a CSV file of counts of each byte seen in each window
* `build_distribution` : Collects counts of each byte seen in a collection of files listed in a single CSV file
* `aggregate_nextbyte_distribution` : Collects counts of each byte that follow prefixes seen in `stdin`.  The results are deposited in a specified directory
* `from_nextbyte_distribution` : Generates stream of bytes to `stdout` given a prefix on `stdin` based on the output of `aggregate_nextbyte_distribution`

## PDF example

As a simple example, the script `pdf_windows.R` and `pdf_windows.csv` shows the byte distribution for a random PDF file [https://github.com/gangtan/LangSec-papers-and-slides/raw/main/langsec22/papers/Robinson_LangSec22.pdf] using
```
./window_distribution 1024 1 256 Robinson_LangSec22.pdf > pdf_windows.csv
```

## Booting examples

Memory captures of a pdp-11/45 at various stages of booting two different operating systems.  A Unibone [https://github.com/j-hoppe/qunibone] was used to capture the memory into the `.dump` files.  Images of memory, clustered with k-means (individually) are shown.  The script `process_dumps.sh` produces the necessary CSV files, and the R scripts produce the graphics.

1. Booting the RUST/SJ pdp-11 operating system [https://github.com/rust11/rust]
2. Booting the ULTRIX-11 pdp-11 operating system using the RL02 images provided by The Unix Historical Society [https://www.tuhs.org/Archive/Distributions/Boot_Images/Ultrix-3.1/]

## The next-byte distribution tools

The next-byte distribution tools are effectively the same idea behind large language models (LLMs), but without any of the neural network machinery.  The are much easier to understand than LLMs.

These tools collect the empircal distribution of tokens that follow a given prefix.  The tradeoff is that they do not scale to handle big prefixes (aka. context windows).  

### Building

Only the standard C libraries are needed to build on Linux:

```
cc bytewise_stats.c aggregate_nextbyte_distribution.c -o aggregate_nextbyte_distribution
cc bytewise_stats.c from_nextbyte_distribution.c -lm -o from_nextbyte_distribution
```

### Training and running

Training data is simply streamed from `stdin`.  You will need a directory where the conditional byte distributions where be stored.  There will generally be many of them, so it is good if the directory is empty first.  Additional runs accumulate into the distributions.

For instance, to use a context window size of 4 bytes (two given 3 bytes, predict 1 byte): 
```
mkdir ~/model
cat somefile.txt | ./aggregate_nextbyte_distribution ~/model 4
```

> [!WARNING]
> Be careful with context window sizes!  Since this is *not* an LLM, large context window sizes can easily result in gigabytes of data being produced!

If you want to generate 1024 byes based on this model, say starting with "foo", you would do
```
echo "foo" | ./from_nextbyte_distribution ~/model 4 1024
```
The output bytes are colored:
* Gray/white: Bytes drawn from the conditional distributions.  Brightness correlates with smaller entropy of the distribution.  White means low entropy, while dark means high entropy.
* Red: Bytes drawn from the distribution of all bytes (no prefix), which happens when a prefix is not found in the training data.

> [!TIP]
> If you wish to disable colors, there is a compile-time flag ANSI_COLOR that can be removed.

It is not required that the context window sizes match.  If there's a mismatch, the content will be cropped accordingly.  In particular, as the model runs, it starts with the stated context window size.  If the current prefix is not found with that size, the prefix is repeatedly cropped (removing bytes from the beginning) until either a match is found or the window is exhausted.  If the window is exhausted, the distribution of all bytes is used as a fallback (output bytes colored red).

### Compressing the model

Given that models can get big, it might be desirable to compress them.  This results in a performance loss, but might be useful if you want to share the model.  Since the programs are quite simplistic and use the filesystem as hash table, you can use [SquashFS](https://docs.kernel.org/filesystems/squashfs.html) to make a read-only compressed copy of the model once trained.  This is much slower but makes the model much more portable.

Continuing the example above,
```
mksquashfs ~/model ~/model.sqfs
mkdir ~/model_smaller
mount ~/model.sqfs ~/model_smaller -t squashfs -o loop
echo "foo" | ./from_nextbyte_distribution ~/model_smaller 4 1024
```

> [!TIP]
> The `mount` command may require `sudo` priviledges.
