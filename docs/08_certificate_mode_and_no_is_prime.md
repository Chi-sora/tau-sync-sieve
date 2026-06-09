# Certificate Mode and `is_prime`-free Validation

Author: Chisora

This document defines the performance model used by the tau-sync sieve and explains when an `is_prime` call is required and when it is not required.

The key distinction is:

```text
raw build mode:
  creates verified facts

certificate mode:
  reuses verified facts
```

## 1. Raw build mode

Raw build mode starts from arithmetic input values.

```text
input:
  N
  stream s
  index j
  q_s(j)
  x_s(N,j) = N - q_s(j)
```

At this stage, the verified tau facts are not yet available.
Therefore, raw build mode may use direct arithmetic tests.

```text
raw build may use:
  is_prime(q_s(j))
  is_prime(x_s(N,j))
  direct semiprime classification
  direct almost-prime classification
  direct TWIN check for x+2
```

Raw build mode outputs verified facts.

```text
raw build output:
  verified tau values
  class labels
  lapse facts
  sync facts
  TWIN companion certificates
```

Raw build mode is not `is_prime`-free.

## 2. Certificate mode

Certificate mode does not create new prime facts.
It checks consistency using already verified facts.

```text
certificate mode input:
  verified tau values
  class labels
  lapse facts
  sync facts
  TWIN companion certificates
```

In certificate mode, covered rows can be classified by lookup.

```text
if tau_only_class = prime:
  x is treated as a verified prime value

if tau_only_class = semiprime:
  x is treated as a verified semiprime value

if tau_only_class = almost_prime:
  x is treated as a verified almost-prime value
```

Thus, for covered certificate rows, the classifier does not call `is_prime(x)`.

```text
certificate classification path:
  x -> verified label lookup -> class
```

This is the `is_prime`-free path.

## 3. What `is_prime`-free means

`is_prime`-free does not mean that no primality test was ever used.
It means that certificate validation does not call `is_prime` for rows already covered by verified facts.

```text
not claimed:
  tau facts appear from nothing
  raw build needs no direct checks

claimed:
  once verified facts exist,
  certificate validation can reuse them without calling is_prime on covered rows
```

## 4. The mod 6 layer

The mod 6 layer is not a tau.
It is a structural residue layer.

```text
q_s(j) = 6j + 1
or
q_s(j) = 6j - 1
```

This layer is used for:

```text
q generation
C3 scan exclusion
origin/orig mod 6 -> B
B -> j_forced
valid_t residue alignment
delta_x integer alignment
```

The tau layer contains only verified first-hit facts.

```text
tau:
  verified first-hit value

mod 6:
  structural residue prefilter
```

Candidate tau is not used.

## 5. Lapse as a composite certificate

For Goldbach endpoint tau, if

```text
tau_gs(N) = t
```

then the prefix before the endpoint is a lapse prefix.

```text
for j < t:
  G_s(N,j) does not hold
```

For q-prime prefix rows, this gives a composite-side certificate for x.

```text
j < tau_gs(N)
and q_s(j) is prime
  -> x_s(N,j) is not prime
```

This is used as a certificate-mode shortcut.

```text
x before tau_g:
  composite by lapse

x at tau_g:
  prime by verified tau_g
```

## 6. Goldbach certificate path

The Goldbach endpoint predicate is:

```text
G_s(N,j):
  q_s(j) is prime
  and
  x_s(N,j) is prime
```

The Goldbach tau is:

```text
tau_gs(N) = min { j : G_s(N,j) holds }
```

In certificate mode, a verified `tau_g` row is a certificate that the endpoint row has:

```text
q prime
x prime
N = q + x
```

For g-sync:

```text
source G
and valid_t
and delta_x = 0
  -> target G
  -> tau_g_target <= j_forced
```

## 7. TWIN companion certificate

The TWIN companion certificate is separate from `tau_g`.

```text
TWIN companion certificate for x:
  x is prime
  and
  x + 2 is prime
```

In certificate mode:

```text
twin_cert = verified certificate value
```

The direct comparison path may check:

```text
is_prime(x + 2)
```

but the tau-only certificate path does not call `is_prime(x+2)` for covered rows.

## 8. Standard direct comparison

For validation against a broadly known direct method, the standard direct classifier is applied to unique values of `x`.

```text
all rows:
  M rows

unique direct check:
  unique x values only
```

This is exact because the arithmetic class of a fixed integer `x` does not depend on how many times it appears.

```text
same x:
  same prime/semiprime/almost-prime class
  same TWIN status
```

Therefore, the direct result can be expanded back to all rows by lookup.

## 9. Speedup definition

The certificate-mode speedup is defined as:

```text
speedup = standard_direct_time / tau_certificate_time
```

where:

```text
standard_direct_time:
  time to classify unique x values by direct arithmetic checks
  plus optional lookup expansion

tau_certificate_time:
  time to validate using verified tau/class/certificate facts
```

Important:

```text
certificate-mode speed is not raw-build speed.
```

It measures how fast already verified facts can be revalidated.

## 10. M = 8388608 actual-value audit

The M=8388608 actual-value audit uses concrete integer values.

```text
M:
  8388608 rows

unique x:
  172364
```

Verified unique classes:

```text
prime:
  110734

semiprime:
  29151

almost_prime:
  32479
```

Expected consistency conditions:

```text
class_mismatch = 0
twin_mismatch = 0
```

TWIN unique-prime result:

```text
unique prime x:
  110734

twin true among unique prime x:
  7045
```

The full-row expansion checks every row by lookup.
If concrete value files are included, they should list the unique `x` values for the prime, semiprime, and almost-prime classes.  Do not name such files as active repository files unless they are present.

## 11. Correct interpretation

The tau-sync sieve uses two different validation levels.

```text
raw build mode:
  direct checks create verified facts

certificate mode:
  verified facts are reused without calling is_prime on covered rows
```

A correct statement is:

```text
The tau-sync sieve supports is_prime-free certificate validation for covered rows.
```

An incorrect statement is:

```text
The tau-sync sieve never needs primality tests.
```

## 12. Summary

```text
mod 6 layer:
  structural residue layer
  not tau

verified tau:
  first-hit facts after verification

certificate mode:
  uses verified facts for fast validation

is_prime-free path:
  applies only to covered certificate rows

standard direct comparison:
  checks unique x values and expands by lookup
```


## 13. Relation to packed-index query

The packed-index query path is a lookup path, not a raw proof of primality from
nothing.

```text
stored record:
  created by build

query:
  reuses stored facts
  may strip factors 2 and 3
  may read one cofactor record
```

If a value is not covered or remains unresolved, the correct output is `U`, not
a guessed class.
