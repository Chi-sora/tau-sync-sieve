# Block Sieve Candidate Generator

## Summary

This file adds the block-sieve optimized candidate generator to `src/`.

## Add to GitHub

Copy:

```text
src/tau_gen_candidates_block.c
scripts/build_block_sieve.bat
scripts/run_block_sieve.bat
scripts/run_block_sieve_noemit.bat
docs/block_sieve_src_note.md
```

## Build

```bat
scripts\build_block_sieve.bat
```

or manually:

```bat
gcc -O3 -std=c11 -Wall -Wextra -march=native -o tau_gen_candidates_block.exe src\tau_gen_candidates_block.c
```

## Run

```bat
tau_gen_candidates_block.exe --steps 10000000000 --filter-primes 10000 --block-size 1048576 --emit out\candidates_block.csv --checkpoint checkpoint\block_state.txt --log logs\block.log --progress-every-blocks 1
```

## Resume

```bat
tau_gen_candidates_block.exe --resume --steps 10000000000 --filter-primes 10000 --block-size 1048576 --emit out\candidates_block.csv --checkpoint checkpoint\block_state.txt --log logs\block.log --progress-every-blocks 1
```

## Speed Test Without CSV

```bat
tau_gen_candidates_block.exe --no-emit --steps 10000000000 --filter-primes 10000 --block-size 1048576 --checkpoint checkpoint\block_noemit_state.txt --log logs\block_noemit.log --progress-every-blocks 1
```

## Known Result

Observed result:

```text
processed=10000000000
k_digits=11
emitted=159845476
filtered=9840154524
rate_steps_per_sec=62927513.80
```

## Correctness Check

The block-sieve output was compared against the previous per-step optimized generator.

Condition:

```text
steps=100000
filter_primes=10000
```

Result:

```text
candidate sequence matched exactly
```
