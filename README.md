# tau-sync index sieve

Author: Chisora

## Status

```text
established:
  finite computational definitions
  arithmetic identities
  validation-scope statements
  packed full-index format specification

not claimed:
  proof of the Goldbach conjecture
  infinite-range theorem
  primality facts created without verification
```

## Purpose

`tau-sync index sieve` documents two connected layers.

```text
tau-sync arithmetic layer:
  q_s(j)
  x_s(N,j)
  tau
  lapse
  sync
  valid_t
  certificates
  no-left-behind rule

packed index layer:
  full-index file from 1 to end
  one byte per integer
  P/S/A/O/U labels
  C0/C2/CM/PR mask
  stored label vs query-resolved label
```

The mathematical layer is for audit and reasoning.  The packed index layer is
for finite lookup and reproducible computation.

## Core warning

```text
mod 6:
  structural residue layer
  not tau

tau:
  verified first-hit fact
```

A residue candidate is not a prime fact and is not a tau value.

## Planned command interface

Code will be added separately.  The planned Windows command interface is:

```bat
build.bat
tau_index.exe build <end> <index.bin> [threads] [chunk_mb]
tau_index.exe query <index.bin> <n>
tau_index.exe twins <index.bin> <start> <end> [count]
```

The packed index covers:

```text
1 <= n <= end
```

This is a full index.  It is not a segmented sieve.

## Packed labels

```text
P:
  prime

S:
  semiprime
  Omega(n) = 2

A:
  almost-prime class
  Omega(n) >= 3

O:
  outside arithmetic domain

U:
  unresolved stored label
```

`AlmostPrimeClass` is project-local terminology.

```text
AlmostPrimeClass(n):
  n is composite
  and n is not semiprime
```

## Packed record

```text
record_size:
  1 byte per integer

bits 0..2:
  stored label

bits 3..6:
  C0/C2/CM/PR mask

bit 7:
  reserved
```

The stored label and the displayed query label are intentionally distinct.

```text
stored_label:
  label written during build

resolved_label:
  label printed by query after optional factor-2/factor-3 cofactor resolution
```

## Documentation

```text
docs/01_definitions.md
docs/02_theory.md
docs/03_validation.md
docs/04_reproducibility.md
docs/05_tau_taxonomy.md
docs/06_outputs_and_labels.md
docs/07_classifier_and_parameters.md
docs/08_certificate_mode_and_no_is_prime.md
docs/09_parallel_tau_lanes.md
docs/10_packed_index_format.md
docs/11_documentation_notes.md
docs/12_tau_c0c2cm_index.md
```

## License

```text
documentation, definitions, theory notes, explanatory text:
  CC BY 4.0

source code:
  MIT
```

See:

```text
LICENSE
CODE-LICENSE-MIT.txt
```

## Included implementation folder

The repository package now includes the current Windows/MinGW index-builder folder.

```text
tau_c0c2cm_index/
  README.txt
  build.bat
  main.c
```

Use this folder for the current packed full-index implementation.

```bat
cd tau_c0c2cm_index
build.bat
tau_index.exe build 60000000 index.bin 8 1024
tau_index.exe query index.bin 10000
tau_index.exe twins index.bin 1 60000000 10
```

The code folder is intentionally kept directly under the repository root.  It is
not placed under `src/` or `scripts/` in this package.

## Author note

God Bless You!
