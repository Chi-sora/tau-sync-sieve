# Strict definitions

Status labels used here:

```text
verified:
  checked in the included finite datasets or scripts

definition:
  adopted notation or formal convention

not claimed:
  not asserted as an infinite theorem
```

## Base objects

Let `N` be the even target value for a stream. Let `s` be a stream.

```text
q_s(j):
  stream prime-side candidate at position j

x_s(N,j):
  N - q_s(j)
```

In the current stream family, q values have the form:

```text
q_s(j) = 6j + 1
or
q_s(j) = 6j - 1
```

This `mod 6` structure is a residue prefilter. It is not a tau.

## Prime number

```text
Prime(n):
  n > 1
  and the only positive divisors of n are 1 and n
```

## Semiprime

```text
Semiprime(x):
  x = p*q
  p and q are prime
  p may equal q
```

## Almost-prime class in this repository

This project uses a local classification convention:

```text
AlmostPrimeClass(x):
  x is composite
  and x is not semiprime
```

This is a project-specific class name. It should not be confused with all conventional uses of the phrase "almost prime" in analytic number theory.

## C3

```text
C3(x):
  x > 3
  and x mod 3 = 0
```

In the scan setting used here:

```text
N = 0 mod 3
q_s(j) = 6j +/- 1
x_s(N,j) = N - q_s(j)
```

Then:

```text
x_s(N,j) != 0 mod 3
```

So C3 does not occur inside `scan_g` or `scan_tv` under this setting.

## Goldbach endpoint

```text
G_s(N,j):
  Prime(q_s(j))
  and Prime(x_s(N,j))
```

Equivalently:

```text
N = q_s(j) + x_s(N,j)
q_s(j) is prime
x_s(N,j) is prime
```

## TWIN companion certificate

```text
TwinCompanion(x):
  Prime(x)
  and Prime(x+2)
```

This is not the same as `tau_g`. It is a companion property of a prime `x`.

## Tau

A tau is a verified first-hit time.

```text
tau_Cs(N):
  min { j : C_s(N,j) holds }
```

Important convention:

```text
candidate residue positions are not tau.
tau is recorded only after the relevant predicate is verified.
```

Examples:

```text
tau_gs(N):
  min { j : G_s(N,j) holds }

tau_semis(N):
  min { j : q_s(j) is prime and x_s(N,j) is semiprime }

tau_furys(N):
  min { j : q_s(j) is prime and x_s(N,j) is AlmostPrimeClass }
```

For the complete list of tau names, component labels, companion certificates, and legacy-name mapping, see:

```text
docs/05_tau_taxonomy.md
```

## Lapse

A lapse is the no-hit prefix associated with a tau.

```text
lapse_Cs(N,J):
  for all j <= J, C_s(N,j) does not hold
```

Relation:

```text
tau_Cs(N) > J
  iff
lapse_Cs(N,J)


tau_Cs(N) = j_star
  iff
lapse_Cs(N,j_star-1)
and
C_s(N,j_star) holds
```

For the complete list of tau names, component labels, companion certificates, and legacy-name mapping, see:

```text
docs/05_tau_taxonomy.md
```

## Lapse decomposition for Goldbach endpoint

For `j < tau_gs(N)`, endpoint failure is decomposed as:

```text
q_fail_s(j):
  q_s(j) is composite

x_killed_lapse_s(N,j):
  q_s(j) is prime
  and x_s(N,j) has a factor p with 5 <= p <= P_def

x_prough_lapse_s(N,j):
  q_s(j) is prime
  and x_s(N,j) has no factor p with 5 <= p <= P_def
```

Thus, within the checked setting:

```text
lapse_gs prefix
  = q_fail
  or x_killed_lapse
  or x_prough_lapse
```

## Sync

For source stream `a` and target stream `b`:

```text
delta_x(j,t):
  x_b(N_b,j) - x_a(N_a,t)
```

A g-sync event is:

```text
G_a(N_a,t)
valid_t(a,b,B,t)
delta_x(j_forced,t) = 0
Prime(q_b(j_forced))
```

Then:

```text
G_b(N_b,j_forced)
tau_gb(N_b) <= j_forced
```

## valid_t

For fixed source, target, and B:

```text
V_B(a,b) = { t :
  Prime(q_a(t))
  and Prime(q_a(t) + B)
}
```

`valid_t` is a verified prime-pair condition, not merely a residue-candidate condition.

## Origin and B

```text
origin:
  original generator state before stream alignment

orig:
  abbreviation of origin

B:
  N_target - N_source
```

In the checked 55/51 -> 15/11 direction, the observed rule was:

```text
orig mod 6 in {0,5}     -> B = 68
orig mod 6 in {1,2,3,4} -> B = 74
```

The full validation used the following stream-pair B values:

```text
11 -> 51: B in {70,76}
15 -> 55: B in {70,76}
51 -> 11: B in {68,74}
55 -> 15: B in {68,74}
```

These B values are finite-validation parameters, not an infinite theorem.


## Classifier and reproducibility parameters

The detailed classifier flow, default parameters (`JMAX_DEFAULT=5000`, `P_def=193`), killed cofactor residual, and value-output requirements are specified in:

```text
docs/07_classifier_and_parameters.md
```
