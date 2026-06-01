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
N = 0 mod 3
q_s(j) = 6j +/- 1

therefore:
  x_s(N,j) = N - q_s(j) is not 0 mod 3
```

So `C3` is inactive inside `scan_g` and `scan_tv` under this setting.

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
