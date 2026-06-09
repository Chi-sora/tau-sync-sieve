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

## Relation to formal definitions

The formal classification structure is documented in:

```text
docs/11_formal_definitions_c0c2cm.md
```

The current implementation should be read as:

```text
packed lookup:
  P/S/A/O/U

route mask:
  C0/C2/CM/PR

safe theorem behind semi/almost:
  S(x) + Exc(x)
```

The one-byte record is not a full theorem witness.  It does not fully store:

```text
S_C0/S_C2/S_CM counts
S_total
sp(x)
Exc witness
P_def=193 killed/P-rough boundary
```

Therefore, if later analysis needs:

```text
S_K/S_PR/A_K/A_PR
tau_shadow/tau_eclipse/tau_fury
```

then it must use sidecar data, extra bits/classes, recomputation, or a trusted
certificate table.

## Endpoint derivation note

The `tau_c0c2cm_index` package is derived from the tau endpoint scan viewpoint.

```text
tau endpoint scan:
  q_s(j)
  x_s(N,j) = N - q_s(j)
  GoldbachHit(N,j,s)
  tau_g(N)
```

Before the first endpoint hit, failed endpoint pairs are audited by endpoint
states and residue-separated composite marks.

```text
endpoint composite route:
  find p >= 5 with p | x
  write x = p * y
  classify the residue pair of (p,y) as C0, C2, or CM
```

This route mark helps compute the implementation quantities:

```text
S_C0(x)
S_C2(x)
S_CM(x)
S_total(x)
```

The safe semi/almost rule is:

```text
S_total + Exc
```

not a residue-only rule.

The packed index stores only the compact result:

```text
P/S/A/O/U + C0/C2/CM/PR
```

It is therefore a compact cache derived from the endpoint/classifier audit
structure, not the full endpoint proof object.

## Lapse and nearend scope

The included `tau_c0c2cm_index` package is a compact packed-index builder.

It does not by itself store a full lapse or nearend tau proof archive.

```text
not fully stored:
  lapse feature witnesses
  exact S_total
  Exc witness
  P_def=193 killed/P-rough split
  nearend tau reconstruction inputs
```

If later analysis needs `tau_shadow`, `tau_eclipse`, `tau_fury`, `S_PR_lapse`,
or `A_PR_lapse`, it must use sidecar data, extra bits/classes, recomputation,
or a trusted certificate table.
