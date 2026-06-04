# 10_Experiment Report

## Overview

This experiment investigated a tau-sync based candidate generator for
twin primes.

The objective was not to prove primality directly, but to determine
whether a lightweight synchronized filter could efficiently eliminate
non-candidates before expensive verification.

------------------------------------------------------------------------

## Basic Definitions

Let

    p0 = 6k - 1
    p1 = 6k + 1

A twin-prime candidate is defined as

    TwinCandidate(k)
    = (6k-1, 6k+1)

The residue filter rejects a candidate if

    6k-1 == 0 (mod r)
    or
    6k+1 == 0 (mod r)

for any cached small prime `r`.

The final twin-prime condition is

    Prime(6k-1)
    and
    Prime(6k+1)

------------------------------------------------------------------------

## Initial Architecture

Generator:

-   Start from k = 1
-   Update residue states
-   Reject obvious composites
-   Save survivors to `candidates.csv`

Verifier:

-   Read candidates
-   Construct
    -   p = 6k - 1
    -   p + 2 = 6k + 1
-   Apply GMP probable-prime test

------------------------------------------------------------------------

## Experimental Results

### Candidate Generation

Observed run:

    step          : about 100,000,000
    k_digits      : 9
    candidate file: about 39 MB
    candidates    : 2,203,806

Approximately 98% of examined states were removed before primality
testing.

------------------------------------------------------------------------

### Twin Prime Verification

Example verified pair:

    (104801, 104803)

The largest verified pair obtained during the experiment was

    (611999681, 611999683)

------------------------------------------------------------------------

## Performance Investigation

CPU utilization remained around 30%.

Several possible bottlenecks were investigated.

### CSV Output

Original version:

-   fopen()
-   fprintf()
-   fclose()

for every candidate.

Optimization:

-   Open file once
-   Large write buffer
-   Periodic flush

Result:

Approximately 1.3x speed improvement.

------------------------------------------------------------------------

### Residue Evaluation Order

Two strategies were compared.

1.  Small primes first
2.  Large primes first

The small-prime-first strategy produced identical results while
requiring far fewer average checks.

Conclusion:

    Early reject using small primes is preferable.

------------------------------------------------------------------------

### Block Sieve Optimization

Observation:

The optimized version still updated every residue at every step.

New approach:

-   Process one block at a time.
-   Mark invalid offsets for every small prime.
-   Update base residues once per block.

Conceptually,

Old:

    O(steps * filter_primes)

New:

    O(block_size * sum(1/r))

Correctness test:

-   steps = 100000
-   filter_primes = 10000

Result:

The generated candidate sets matched exactly.

------------------------------------------------------------------------

## Discussion

The experiment suggests that the major strength of tau-sync is not
proving primality directly.

Its primary value appears to be:

-   candidate compression
-   synchronization-based filtering
-   reduction of expensive primality tests

The search space itself remains extremely large, but the filtering stage
is computationally lightweight.

------------------------------------------------------------------------

## Future Work

Possible improvements:

1.  Better integration between residue logic and tau-sync.
2.  Remove unnecessary residue calculations.
3.  Dynamic ordering based on rejection frequency.
4.  Binary output instead of CSV.
5.  Block-based parallel generation.

------------------------------------------------------------------------

## Conclusion

The experiments demonstrated that:

-   tau-sync style filtering is computationally inexpensive,
-   candidate reduction is significant,
-   block-sieve style optimization preserves correctness,
-   and the overall approach scales well for long-running searches.

The algorithm appears to function more naturally as a high-speed
candidate generator than as a direct primality prover.
