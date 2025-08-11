# bytewise-stats: Looking a statistics of byte usage in files

## Six utilities are provided:
* `window_distribution` : runs a sliding window over a file, producing a CSV file of counts of each byte seen in each window
* `build_distribution` : Collects counts of each byte seen in a collection of files listed in a single CSV file
* `aggregate_nextbyte_distribution` : Collects counts of each byte that follow prefixes seen in `stdin`.  The results are deposited in a specified directory
* `from_nextbyte_distribution` : Generates stream of bytes to `stdout` given a prefix on `stdin` based on the output of `aggregate_nextbyte_distribution`
* `compare_nextbyte_distributions` : Runs Chi^2 goodness of fit test for two next byte reports produced by `aggregate_nextbyte_distribution`
* `compare_byte_distributions` : Runs Chi^2 goodness of fit test for two byte histogram files, each of which was one of the files made by `aggregate_nextbyte_distribution`

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

The next-byte distribution tools are effectively the same idea behind large language models (LLMs), but without any of the neural network machinery.  The are much easier to understand than LLMs.  They also admit rigorous statistical tests.

These tools collect the empircal distribution of tokens that follow a given prefix.  The tradeoff is that they do not scale to handle big prefixes (aka. context windows).  

### Building

Only the standard C libraries are needed to build on Linux:

```
cc bytewise_stats.c aggregate_nextbyte_distribution.c -o aggregate_nextbyte_distribution
cc bytewise_stats.c from_nextbyte_distribution.c -lm -DANSI_COLOR -o from_nextbyte_distribution
cc bytewise_stats.c compare_byte_distribution.c -lm -o compare_byte_distribution
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
> If you wish to disable colors, there is a compile-time flag ANSI_COLOR in the above build command that can be removed.

It is not required that the context window sizes match.  If there's a mismatch, the content will be cropped accordingly.  In particular, as the model runs, it starts with the stated context window size.  If the current prefix is not found with that size, the prefix is repeatedly cropped (removing bytes from the beginning) until either a match is found or the window is exhausted.  If the window is exhausted, the distribution of all bytes is used as a fallback (output bytes colored red).  Fallbacks should be very rare except when starting off, since no approximations are being used.

### Comparing models

Once you have run `aggregate_nextbyte_distribution` on two different datasets, you can compare the distributions that it produces.  This can be done for an individual pair of histogram files or in bulk.

The Chi^2 test asks whether a byte stream may have been produced according to a given distribution.  The byte stream needs to be transformed into a byte histogram (via `aggregate_nextbyte_distribution`).  This is called the *test* histogram.
The test histogram is to be compared to an existing histogram (which may also have been made by `aggregate_nextbyte_histogram`), called the *reference* histogram.

#### Running for a single distribution

So, if you want to ask whether a single histogram from the test data could have come from the reference data's distribution, you run the following:

```
compare_byte_histograms ~/model_reference/null ~/model_test/null
```

This produces four comma-separated numbers:  

1. "reference_count" : how many total instances of this prefix were found in the reference distribution
2. "test_count" : how many total instances of this prefix were found in the test distribution
3. "chisq" : the resulting chi^2 value for the test 
4. "pvalue" : the resulting p-value for the test. Smaller p-values mean that the distributions are more different.

> [!TIP]
> The Chi^2 test is not commutative; running the arguments in the opposite order will give different results.

#### Running against all distributions

If you want to check all next byte conditional distributions to see if a given byte stream came from a given model (via `from_nextbyte_distribution`), there is a convenience script to aggregate the results.

```
mkdir ~/test_index
cat file_of_interest | ./aggregate_nextbyte_distribution ~/test_index 4
sh compare_nextbyte_distributions ~/model_reference ~/test_index > results.csv
```

The resulting CSV file has the following columns:

1. "prefix" : The hex string prefix for the conditional histogram being tested
2. "reference_count" : how many total instances of this prefix were found in the reference data
3. "test_count" : how many total instances of this prefix were found in the test data
4. "chisq" : the resulting chi^2 value for the test
5. "pvalue" : the resulting p-value for the test

If either the "test_count" or "reference_count" values are small, the test is likely to be unreliable.

> [!WARNING]
> Do not just take the minimum p-value to see whether the overall stream test fails!
> Since there are multiple tests being performed, the p-values should be adjusted accordingly.
> See [https://en.wikipedia.org/wiki/Multiple_comparisons_problem#Controlling_procedures]

#### Case study: verifying the output of `from_nextbyte_distribution`

Let's check that `from_nextbyte_distribution` really does what it says:

```
mkdir ~/jnkindex
echo "chap" | ./from_nextbyte_distribution ~/indexpath 4 1024 | tee "testfile" | ./aggregate_nextbyte_distribution ~/jnkindex 4
sh ./compare_nextbyte_distributions ~/indexpath ~/jnkindex | awk -F',' '$3 > 3 && $5 < 1e-3'
```

This checks to see if there are any prefixes for which the test file has more than 3 instances in which the p-value is less than 1e-3.  For a short file (1024 bytes), there should not be any, so the last line should not return anything!  If it does, you can investigate the `testfile` that is produced on the second line.

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
