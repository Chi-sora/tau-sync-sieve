# Packed full-index file format

## Purpose

This document defines the planned packed index file format.

## File type

```text
type:
  full index

range:
  1 <= n <= end

not:
  segmented index
```

The file stores one byte for each integer in the range.

## Header

```text
header_size:
  64 bytes

encoding:
  little-endian

magic:
  TSC2CM3

version:
  3

record_size:
  1 byte
```

Header fields should be written manually.  Do not write a C struct directly into
the header buffer.

Unsafe pattern:

```text
memcpy(buf, &header_struct, sizeof(header_struct))
```

Reason:

```text
C struct padding can make sizeof(header_struct) larger than 64 bytes
```

## Offset formula

For the current builder:

```text
start = 1
RECORD_SIZE = 1
```

Therefore:

```text
offset(n) = 64 + (n - 1)
```

General form:

```text
offset(n) = INDEX_HEADER_SIZE + (n - start) * RECORD_SIZE
```

## Record byte

```text
bits 0..2:
  label

bits 3..6:
  mask

bit 7:
  reserved
```

Label values:

```text
0:
  O

1:
  P

2:
  S

3:
  A

4:
  U
```

Mask values:

```text
bit 3:
  C0

bit 4:
  C2

bit 5:
  CM

bit 6:
  PR
```

## Stored label and resolved label

```text
stored_label:
  label in the record byte

resolved_label:
  label printed by query
```

The builder may store:

```text
stored_label = U
mask includes PR
```

when completing the classification would require a predecessor/cofactor lookup
that would cause random disk I/O during build.

The query command may resolve this by:

```text
1. remove factors 2 and 3 from n
2. read one cofactor record
3. combine Omega contribution from 2 and 3 with the cofactor label
```

## Example: n = 10000

```text
10000 = 2^4 * 5^4
Omega(10000) = 8
```

The query-resolved output is:

```text
label = A
label_long = ALMOSTPRIME
```

The stored output may be:

```text
stored_label = U
stored_resmask = PR
```

depending on the build strategy.

## Build I/O rule

The builder should avoid random reads from the already written predecessor part
of `index.bin`.

```text
preferred build:
  sequential chunk write
  no predecessor random read

query:
  small number of random reads is acceptable
```

This is why stored and resolved labels are distinct.
## P_def boundary warning

The `PR` mask bit is compact diagnostic information.  It must not be read as a
complete replacement for the historical `P_def=193` killed/P-rough split.

```text
RES_PR:
  packed-index route marker

P_ROUGH(x):
  no prime p with 5 <= p <= 193 divides x
```

These are related ideas, but they are not the same datum.

The packed index can support resolved arithmetic labels:

```text
P
S
A
```

However, nearend tau reconstruction may require:

```text
S_K
S_PR
A_K
A_PR
```

Those labels require the `P_def=193` killed/P-rough boundary.  Store it in a
sidecar, add explicit bits, recompute it during nearend audit, or use a trusted
certificate table.

## Included implementation path

The included implementation folder is:

```text
tau_c0c2cm_index/
```

The implementation files are kept directly under that folder.

```text
tau_c0c2cm_index/main.c
tau_c0c2cm_index/build.bat
tau_c0c2cm_index/README.txt
```

The current implementation corresponds to this format document:

```text
magic:
  TSC2CM3

version:
  3

record_size:
  1 byte
```

If `main.c` changes these values, this document must be updated in the same commit.

## Formal-classification storage limits

The packed record is intentionally compact.

```text
stored:
  P/S/A/O/U label
  C0/C2/CM/PR mask

not fully stored:
  S_C0(x)
  S_C2(x)
  S_CM(x)
  S_total(x)
  sp(x)
  Exc(x)
  P_def=193 killed/P-rough split
```

The safe theorem route is documented in:

```text
docs/11_formal_definitions_c0c2cm.md
```

Summary:

```text
safe theorem:
  S(x) + Exc(x)

implementation:
  S_C0/S_C2/S_CM

diagnostics:
  C0/C2/CM/PR mask
```

A query may resolve `P/S/A` for many values, but the one-byte record alone
should not be treated as a complete proof object for all diagnostic or nearend
tau labels.
