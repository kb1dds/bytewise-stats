# bytewise-stats: Looking a statistics of byte usage in files

## Four utilities are provided:
* `window_distribution` : runs a sliding window over a file, producing a CSV file of counts of each byte seen in each window
* `build_distribution` : Collects counts of each byte seen in a collection of files listed in a single CSV file
* `aggregate_nextbyte_distribution` : Collects counts of each byte that follow prefixes seen in `stdin`.  The results are deposited in a specified directory
* `from_next_distribution` : Generates stream of bytes to `stdout` given a prefix on `stdin` based on the output of `aggregate_nextbyte_distribution`

## PDF example

As a simple example, the script `pdf_windows.R` and `pdf_windows.csv` shows the byte distribution for a random PDF file [https://github.com/gangtan/LangSec-papers-and-slides/raw/main/langsec22/papers/Robinson_LangSec22.pdf] using
```
$ ./window_distribution 1024 1 256 Robinson_LangSec22.pdf > pdf_windows.csv
```

## Booting examples

Memory captures of a pdp-11/45 at various stages of booting two different operating systems.  A Unibone [https://github.com/j-hoppe/qunibone] was used to capture the memory into the `.dump` files.  Images of memory, clustered with k-means (individually) are shown.  The script `process_dumps.sh` produces the necessary CSV files, and the R scripts produce the graphics.

1. Booting the RUST/SJ pdp-11 operating system [https://github.com/rust11/rust]
2. Booting the ULTRIX-11 pdp-11 operating system using the RL02 images provided by The Unix Historical Society [https://www.tuhs.org/Archive/Distributions/Boot_Images/Ultrix-3.1/]