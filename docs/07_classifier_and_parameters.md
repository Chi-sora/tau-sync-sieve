# Classifier, parameters, and finite-validation scope

This document records implementation-level definitions that are used by the included reproducibility tools.
They are part of the repository specification, but they are not claimed as infinite number-theoretic theorems.

## 1. Fixed default parameters

```text
JMAX_DEFAULT:
  5000

P_def:
  193
```

`JMAX_DEFAULT` is the default maximum stream index scanned by the included C tools.

`P_def` is the small-prime cutoff used by the killed / P-rough classifier.

These are reproducibility parameters, not intrinsic constants of the tau-sync sieve framework.

## 2. Residue layer

The checked stream family uses:

```text
q_s(j) = 6j + 1
or
q_s(j) = 6j - 1
```

This layer is structural. It is not a tau.

Consequences used by the implementation:

```text
x_s(N,j) = N - q_s(j)

C3_s(N,j) occurs
  iff x_s(N,j) = 0 mod 3
  iff N = q_s(j) mod 3
```

Therefore, inside `scan_g` and `scan_tv`, `C3` is inactive whenever

```text
N != q_s(j) mod 3
```

The checked tau-sync sieve configuration satisfies this residue separation
condition by construction. The case `N = 0 mod 3` and `q_s(j)=6j +/- 1` is only
a sufficient example, not the general reason.

## 3. Classifier output classes

For an integer `x`, the project classifier returns one of the following class labels.

```text
P:
  x is a prime number

C3:
  x > 3 and x mod 3 = 0

S_PR:
  x is P-rough semiprime

S_K:
  x is killed semiprime

A_PR:
  x is P-rough almost-prime class

A_K:
  x is killed almost-prime class
```

Here:

```text
Semiprime(x):
  x = p*q, where p and q are prime and p may equal q

AlmostPrimeClass(x):
  x is composite and not semiprime

KILLED(x):
  exists prime p with 5 <= p <= P_def and p divides x

P_ROUGH(x):
  no prime p with 5 <= p <= P_def divides x
```

The term `almost-prime class` is project-local. It is not intended to replace standard analytic-number-theory terminology.

## 4. classify5 flow

The included classifier follows this order.

```text
classify5(x):

  1. If C3(x), return C3.

  2. If x has a small prime factor p with 5 <= p <= P_def:
       y = x / p

       if y is prime:
         return S_K
       else:
         return A_K

  3. If x is prime:
       return P

  4. Otherwise x is P-rough composite.
     Use cbrt trial division and class checks:

       if x is semiprime:
         return S_PR
       else:
         return A_PR
```

The classifier does not require Pollard rho or SQUFOF in the included validation path.

## 5. killed cofactor residual

For killed cases:

```text
x = p*y
5 <= p <= P_def
p divides x
```

The remaining hard distinction is whether `y` is prime.

```text
if y is prime:
  x is S_K

if y is composite:
  x is A_K
```

When `y` is P-rough and has no cbrt factor, tau and cbrt information alone do not distinguish `y prime` from `y semiprime`.
In that residual, `is_prime(y)` or a trusted certificate is required.

Finite checked residual from development data:

```text
residual = 285 / 1480 = 19.3 percent

inside residual:
  prime cofactor     = 59.6 percent -> S_K
  semiprime cofactor = 40.4 percent -> A_K
```

## 6. Prime facts and certificates

A verified tau is not the same as a residue candidate.

```text
verified tau:
  recorded only after the relevant predicate has been verified

certificate mode:
  reuses verified tau, lapse, class, and companion facts

raw build mode:
  creates verified facts from direct checks or trusted certificates
```

A TWIN companion certificate is:

```text
TwinCompanion(x):
  Prime(x) and Prime(x+2)
```

This certificate is not a tau value.

## 7. Goldbach HIT value output

For auditability, Goldbach HIT value outputs should include numeric values:

```text
N
j
q
x
x_plus_2
goldbach_check
twin_cert
```

The intended check is:

```text
goldbach_check = 1
  iff
N = q + x
and q is verified prime
and x is verified prime
```

For semiprime and almost-prime class audits, outputs should include at least:

```text
x
tau_only_class
standard_direct_class
match
occurrence_count
```

## 8. Scope warning

The included validations are finite computational checks.
They support the repository definitions and checked datasets, but they do not constitute an infinite theorem or a proof of the Goldbach conjecture.


## 13. Packed-index relation to classify5

The packed index builder is not required to run the historical `classify5` flow
for every integer during build.

The current performance-oriented rule is:

```text
build:
  avoid random predecessor reads
  store unresolved factor-2/factor-3 cofactor cases as U when needed
  mark PR when the P-rough or cofactor route matters

query:
  strip factors 2 and 3
  read one cofactor record when needed
  display the resolved label
```

This keeps large builds closer to sequential write plus CPU marking.

## 14. S_PR and PR bit

If semiprime or almost-prime information is reached through a P-rough route or
a factor-2/factor-3 cofactor route, the packed index uses:

```text
PR bit:
  set in the mask
```

This prevents `S_PR`-like cases from becoming indistinguishable from
`resmask = 0`.
## Packed-index sieve vs classify5

The historical `classify5` definition and the packed-index sieve do not record
exactly the same diagnostic split.

### classify5 diagnostic split

```text
P_def:
  193
```

The historical classifier uses `P_def=193` to separate killed and P-rough
classes.

```text
KILLED(x):
  exists prime p with 5 <= p <= P_def and p divides x

P_ROUGH(x):
  no prime p with 5 <= p <= P_def divides x
```

Therefore the historical nearend labels distinguish:

```text
S_K:
  killed semiprime

S_PR:
  P-rough semiprime

A_K:
  killed almost-prime class

A_PR:
  P-rough almost-prime class
```

### packed-index sieve split

The packed-index builder uses sieve information over prime divisors for the
stored `P/S/A/O/U` label and records only a compact mask.

```text
stored label:
  P/S/A/O/U

mask:
  C0/C2/CM/PR
```

The packed index gives the same arithmetic `P/S/A` class when the value is
resolved, but it does not by itself preserve the `P_def=193` boundary.

Important distinction:

```text
RES_PR:
  packed-index route marker

RES_PR is not identical to:
  P_ROUGH(x) with P_def=193
```

### consequence for nearend tau

The following nearend objects require the killed/P-rough split with the declared
`P_def` boundary.

```text
tau_shadow
tau_eclipse
tau_fury
S_PR_lapse
A_PR_lapse
```

Therefore, if these nearend tau values are computed after building a packed
index, one of the following is required.

```text
1. store a separate killed/P-rough sidecar field,
2. add a dedicated packed bit or class extension for the P_def boundary,
3. recompute the P_def=193 killed/P-rough split during the nearend audit,
4. use a trusted certificate table that contains the split.
```

Without one of these, the packed index supports the arithmetic class:

```text
P/S/A
```

but does not fully support reconstruction of:

```text
S_K vs S_PR
A_K vs A_PR
```

This limitation is diagnostic, not a contradiction in the arithmetic
prime/semiprime/almost-prime classification.
