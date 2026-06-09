# Outputs, labels, and interpretation guide

This file explains the main CSV outputs and how to read them.

## 1. Classification outputs

Typical columns:

```text
x:
  integer being classified

tau_only_class:
  class obtained from verified tau / lapse / certificate facts

standard_direct_class:
  class obtained from direct standard checks in the included scripts

match:
  1 if tau_only_class equals standard_direct_class
  0 otherwise

occurrence_count:
  number of rows in the M-level table where this x occurs
```

Classes:

```text
prime:
  x is a prime number

semiprime:
  x = p*q, p and q prime, p may equal q

almost_prime:
  project-local class: composite and not semiprime
```

## 2. TWIN outputs

Typical columns:

```text
twin_cert:
  TWIN companion certificate result

twin_direct:
  direct check of Prime(x+2)

twin_match:
  1 if twin_cert equals twin_direct
```

TWIN means:

```text
Prime(x) and Prime(x+2)
```

TWIN is a companion property of prime `x`. It is not the same as a Goldbach endpoint.

## 3. Goldbach HIT value outputs

Goldbach HIT tables should include actual numeric values:

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

Interpretation:

```text
goldbach_check = 1
  means N = q + x and both q and x are verified prime facts in the certificate data
```

## 4. Full sync validation outputs

Important metrics:

```text
source_cases:
  number of source-side cases considered

sync_slots:
  number of valid sync candidate slots

sync_success:
  number of successful g-sync transfers

sync_fail:
  should be 0 in the included validation

delta_fail:
  should be 0 when delta_x=0 alignment is correct

invalid_j:
  should be 0 when forced positions are valid

target_direct_fail:
  should be 0 when direct target audit is enabled
```

## 5. Certificate mode vs raw build mode

```text
raw build mode:
  creates verified tau/class/certificate facts from direct checks

certificate mode:
  reuses existing verified facts for fast validation
```

Do not interpret certificate-mode speed as raw construction speed.

## 6. Large CSV policy

The repository should keep only small samples and summary outputs.
Large all-row CSV files, binary caches, and full raw outputs should be kept outside the repository or attached to releases if needed.


## 7. Packed-index query output

The packed index query should distinguish stored and resolved values.

```text
n:
  requested integer

range:
  index coverage interval

label:
  query-resolved label

label_long:
  expanded name of the query-resolved label

resmask:
  query-resolved or inherited C0/C2/CM/PR mask

stored_label:
  raw label from the one-byte record

stored_resmask:
  raw mask from the one-byte record
```

Interpretation:

```text
stored_label:
  build-time state

label:
  query-time resolved state
```

If `stored_label = U`, query may still print `label = P`, `S`, or `A` after
factor-2/factor-3 cofactor resolution.

## 8. Active file-reference policy

Do not list large CSV files, result files, or generated binary caches as active
repository files unless they are present.  It is acceptable to keep summary
values as finite validation summaries.
