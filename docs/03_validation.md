# Validation summary

This document reports finite computational validation only. It does not assert an infinite theorem.

## Full sync validation

Historical summary values:

Main values:

```text
source_cases = 335544320
sync_slots   = 200371531
sync_success = 200371531
sync_fail    = 0
invalid_j    = 0
delta_fail   = 0
target_direct_fail = 0
```

Stream-pair checked family:

```text
11 -> 51
15 -> 55
51 -> 11
55 -> 15
```

B values checked by stream pair:

```text
11 -> 51: B = 70, 76
15 -> 55: B = 70, 76
51 -> 11: B = 68, 74
55 -> 15: B = 68, 74
```

## Lapse and C3

C3 scan count:

```text
C3_scan = 0
```

This matches the mod 6 arithmetic exclusion.

The q-prime prefix x-side split stayed close to:

```text
x_killed  about 73.4 percent
x_prough  about 26.6 percent
```

The x_prough split stayed close to:

```text
S_PR  about 57 percent
A_PR  about 43 percent
```

## M=8388608 classification validation

Historical summary values:

Unique x values:

```text
unique all   = 172364
prime        = 110734
semiprime    = 29151
almost_prime = 32479
mismatch     = 0
```

Uploaded-value recheck summary:

Summary:

```text
ALL rows   = 172364
unique_x   = 172364
mismatch   = 0
```

Overlap check:

```text
prime vs semiprime       overlap = 0
prime vs almost_prime    overlap = 0
semiprime vs almost_prime overlap = 0
```

## Goldbach HIT values

Historical summary values:

Main values:

```text
full event rows      = 983850
unique HIT prime x   = 110734
goldbach_fail        = 0
twin_unknown         = 0
```

The full event output used during development included:

```text
N
j
q
x
x_plus_2
q_prime_by_tau
x_prime_by_tau
goldbach_check
twin_cert
```

## TWIN companion validation

Historical summary values:

TWIN companion certificate:

```text
Prime(x) and Prime(x+2)
```

Checked values:

```text
prime x       = 110734
covered       = 110734
unknown       = 0
twin true     = 7045
validation fail = 0
```

## Important interpretation

`tau-only certificate mode` means:

```text
verified tau/lapse/class/certificate facts are already available
```

It is not the same as raw first construction. Raw construction still requires direct verification or a trusted certificate source.


## Packed-index validation targets

For the current packed-index format, validation should include:

```text
header_valid:
  magic/version/record_size match the expected format

offset_valid:
  offset(n) = 64 + (n - 1)

stored_vs_resolved_valid:
  stored_label and query resolved label are not confused

no_predecessor_random_read_in_build:
  build avoids random reads from already written predecessor records

query_resolution_valid:
  factor-2/factor-3 cofactor resolution gives the expected displayed label
```

Example target:

```text
n = 10000
10000 = 2^4 * 5^4
Omega(10000) = 8
resolved label = A
```

## File-reference policy

This document may keep numerical summaries from previous validation work.
Do not reference result CSV files as active repository files unless those files
are actually present in the repository.
