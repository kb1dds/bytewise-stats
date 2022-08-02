# bytewise-stats: Looking a statistics of byte usage in files

## Two utilities are provided:
* `window_distribution` : runs a sliding window over a file, producing a CSV file of counts of each byte seen in each window
* `build_distribution` : Collects counts of each byte seen in a collection of files listed in a single CSV file

## Two examples are provided:

Memory captures of a pdp-11/45 at various stages of booting two different operating systems.  A Unibone [https://github.com/j-hoppe/qunibone] was used to capture the memory into the `.dump` files.  Images of memory, clustered with k-means (individually) are shown.

1. Booting the RUST/SJ pdp-11 operating system [https://github.com/rust11/rust]
2. Booting the ULTRIX-11 pdp-11 operating system using the RL02 images provided by The Unix Historical Society [https://www.tuhs.org/Archive/Distributions/Boot_Images/Ultrix-3.1/]