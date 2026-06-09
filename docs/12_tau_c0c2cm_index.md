# tau_c0c2cm_index implementation folder

## Purpose

This folder contains the current Windows/MinGW implementation package for the
packed full-index builder.

```text
tau_c0c2cm_index/
  README.txt
  build.bat
  main.c
```

## Scope

```text
established:
  finite index builder package

not claimed:
  infinite theorem
  proof of the Goldbach conjecture
```

## Build

```bat
cd tau_c0c2cm_index
build.bat
```

The build creates:

```text
tau_index.exe
```

## Commands

```bat
tau_index.exe build <end> <index.bin> [threads] [chunk_mb]
tau_index.exe query <index.bin> <n>
tau_index.exe twins <index.bin> <start> <end> [count]
```

## Build example

```bat
tau_index.exe build 60000000 index.bin 8 1024
```

## Query example

```bat
tau_index.exe query index.bin 10000
```

Expected arithmetic interpretation:

```text
10000 = 2^4 * 5^4
Omega(10000) = 8
resolved label = A
```

## Twin search example

```bat
tau_index.exe twins index.bin 1 60000000 10
```

## Index file

```text
range:
  1 <= n <= end

type:
  full index

record_size:
  1 byte per integer

header:
  64 bytes
```

Disk formula:

```text
index_size_bytes = 64 + end
```

## Stored label and resolved label

```text
stored_label:
  build-time label stored in the record

resolved_label:
  query-time label after optional factor-2/factor-3 cofactor resolution
```

`U` is an explicit storage state.

```text
U must not be treated as P, S, or A without query or certificate resolution.
```

## P_def warning

The implementation stores a compact `PR` route marker.  This marker is not
identical to the historical `P_ROUGH(x)` predicate with `P_def=193`.

```text
RES_PR:
  packed-index route marker

P_ROUGH(x):
  no prime p with 5 <= p <= 193 divides x
```

Therefore, nearend tau reconstruction for labels such as:

```text
S_K
S_PR
A_K
A_PR
tau_shadow
tau_eclipse
tau_fury
```

requires a sidecar, extra bits, recomputation, or a trusted certificate table.
