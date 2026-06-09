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

## Current command interface

The current Windows/MinGW implementation is included in `tau_c0c2cm_index/`.  The command interface is:

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

## C0/C2/CM formal clarification

The derivation path is:

```text
tau endpoint scan
  -> endpoint composite marks
  -> C0/C2/CM residue-separated implementation
  -> S_total + Exc
  -> packed index label and mask
```

```text
safe theorem:
  S(x) + Exc(x)

implementation:
  S_C0(x), S_C2(x), S_CM(x)

diagnostics:
  C0/C2/CM route marks and lapse features
```

`C0/C2/CM` is the residue-separated implementation and diagnostic layer.  It is
not, by itself, the full theorem witness.  The current one-byte packed record
does not store exact `S_total`, `sp(x)`, `Exc(x)`, or the `P_def=193`
killed/P-rough split.

See:

```text
docs/11_formal_definitions_c0c2cm.md
```

## Lapse and diagnostic safety

```text
lapse:
  downstream of tau_g

C0/C2/CM lapse features:
  diagnostic and priority features

window priority:
  may reorder candidates
  must not delete candidates without certificate
```

The current one-byte packed index is not a full lapse/nearend tau proof archive.
See:

```text
docs/11_formal_definitions_c0c2cm.md
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
docs/08_packed_index_format.md
docs/09_documentation_notes.md
docs/10_tau_c0c2cm_index.md
docs/11_formal_definitions_c0c2cm.md
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
