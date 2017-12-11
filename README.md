## Installation

1. Clone the repo
2. Run gsl-install.sh
3. Compile with `make`
4. Run

## Notes

To help verify correctness, we have set the RNG seeds to a constant.  The md5sum of each versions output is compared to that of the original serial version.  For the "tiny" example, all md5sums of the output from each binary match (00 thru 02).  For "small" some rounding error in 01-omp-verlet and 02-omp-verlet-interval is causing a small difference in the output files and different hashes as a result.  Seeing that `head -n 50000` renders a match, we are satisfied that the program is still correct.

## Roadmap

- Implement serial cell lists with radius padding
- Add OpenMP directives to verlet + cell list to make it go fast
- Introduce CUDA
