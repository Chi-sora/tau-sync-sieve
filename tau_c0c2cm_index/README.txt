tau_c0c2cm_index

Status:
  finite computational index builder
  not an infinite theorem
  not a proof of Goldbach's conjecture

Environment:
  Windows 11
  MinGW-w64 gcc
  cmd.exe

Files:
  main.c
  build.bat
  README.txt

Build:
  build.bat

Executable:
  tau_index.exe

Commands:
  tau_index.exe build <end> <index.bin> [threads] [chunk_mb]
  tau_index.exe query <index.bin> <n>
  tau_index.exe twins <index.bin> <start> <end> [count]

Examples:
  tau_index.exe build 60000000 index.bin 8 1024
  tau_index.exe query index.bin 10000
  tau_index.exe twins index.bin 1 60000000 10

Large example:
  tau_index.exe build 100000000000 index.bin 16 1024

Index range:
  build creates records for:
    1 <= n <= end

  The lower bound 1 is fixed.
  Do not pass 1 to the build command.

Index type:
  full index
  one record per integer
  not a segmented sieve

Index format:
  magic:
    TSC2CM3

  version:
    3

  header:
    64 bytes
    little-endian manual encoding

  record size:
    1 byte per integer

Offset formula:
  offset(n) = 64 + (n - 1)

Record layout:
  bits 0..2:
    stored label

  bits 3..6:
    resmask C0/C2/CM/PR

  bit 7:
    reserved

Stored labels:
  O:
    outside

  P:
    prime

  S:
    semiprime
    Omega(n) = 2

  A:
    almost-prime class
    Omega(n) >= 3

  U:
    unresolved stored label

Resolved label:
  query prints the resolved label.

  If stored_label is U, query may:
    1. strip factors 2 and 3 from n,
    2. read one cofactor record,
    3. combine Omega contribution,
    4. print resolved label P/S/A/O/U.

Example:
  n = 10000
  10000 = 2^4 * 5^4
  Omega(10000) = 8
  resolved label = A

Mask:
  C0:
    C0 residue/component route

  C2:
    C2 residue/component route

  CM:
    mixed residue/component route

  PR:
    packed-index route marker for P-rough or cofactor-resolution path

Important P_def warning:
  Historical classify5 uses:
    P_def = 193

  classify5 distinguishes:
    S_K vs S_PR
    A_K vs A_PR

  This packed index stores:
    P/S/A/O/U
    C0/C2/CM/PR

  RES_PR is not identical to:
    P_ROUGH(n) with P_def = 193

  Therefore this index supports arithmetic P/S/A lookup, but does not by itself
  fully reconstruct:
    S_K vs S_PR
    A_K vs A_PR

  For tau_shadow, tau_eclipse, tau_fury, S_PR_lapse, or A_PR_lapse, use one of:
    1. sidecar field for killed/P-rough at P_def=193,
    2. explicit extra packed bits/classes,
    3. recomputation during nearend audit,
    4. trusted certificate table.

Build design:
  build avoids random reads from already written predecessor records.
  lane records are finalized and written directly.
  no second chunk-sized combined buffer is used.

Progress:
  dedicated progress thread
  prints:
    [progress] xx.xx% (done/total)

  intended interval:
    about 5 seconds

Build summary:
  build prints:
    [build] stored_u_count=...

Meaning:
  stored_u_count is the number of records stored as U.
  U is allowed as a storage state.
  U must not be treated as P, S, or A without query/certificate resolution.

Twin search:
  twins uses block reads rather than one seek per candidate.

Disk size:
  index_size_bytes = 64 + end

Example for end = 100000000000:
  index_size_bytes = 100000000064
  approximately 93.13 GiB

Reproducibility cautions:
  1. Extract the zip before running build.bat.
  2. Rebuild after replacing main.c.
  3. Do not mix old executable with new source.
  4. Do not mix old index format with new executable.
  5. Do not open index.bin in another program while building.
  6. Keep enough free disk space.

Known scope:
  finite computation only
  no infinite theorem claimed
  no proof of Goldbach conjecture claimed


Formal definition reference:
  docs/11_formal_definitions_c0c2cm.md

Important:
  C0/C2/CM is an implementation and diagnostic mask layer.
  The safe theorem route is S(x)+Exc(x).
  The one-byte record does not fully store S_total, sp(x), Exc(x), or
  the P_def=193 killed/P-rough split.


Derivation path:
  tau endpoint scan
  -> endpoint composite marks
  -> C0/C2/CM residue-separated implementation
  -> S_total + Exc classifier
  -> packed index label and mask

Meaning:
  tau endpoint scan supplies the operational endpoint values q and x.
  C0/C2/CM records residue-separated composite routes for endpoint values.
  The safe theorem route is S_total + Exc.
  The packed index stores the compact result/cache, not the full proof object.


Lapse and nearend scope:
  The packed index is not a full lapse or nearend tau proof archive.

  Diagnostic features may be used for:
    long-tail analysis
    pair_CC
    q_any
    x_any
    pair_anyany
    window priority

  They must not be used to delete candidates without certificate.

  Required invariants:
    false_skip_j = 0
    P_UNKNOWN_used_as_prime = 0
