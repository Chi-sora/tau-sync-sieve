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

In a scan stream, C3 is governed by the residue separation condition:

```text
x_s(N,j) = N - q_s(j)

C3_s(N,j) occurs
  iff x_s(N,j) = 0 mod 3
  iff N = q_s(j) mod 3
```

Therefore:

```text
C3_s(N,j) is inactive
  iff N != q_s(j) mod 3
```

In the checked tau-sync sieve configuration, the stream residues are chosen so that
`N != q_s(j) mod 3` at the scanned indices. Hence C3 does not occur inside
`scan_g` or `scan_tv` under that configuration.

The special case `N = 0 mod 3` and `q_s(j)=6j +/- 1` is one sufficient example,
but it is not the general reason for C3 inactivity.

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
  min { j >= 1 : C_s(N,j) holds }   if such j exists
  infinity                           otherwise
```

The infinity convention makes tau total. All relations below hold with
this convention, including the no-hit case.

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

lapse_Cs(N,0):
  true   (empty prefix convention)
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
  and x_s(N,j) has a prime factor p with 5 <= p <= P_def

x_prough_lapse_s(N,j):
  q_s(j) is prime
  and x_s(N,j) has no prime factor p with 5 <= p <= P_def
```

The decomposition is exhaustive and exclusive over the lapse prefix
under the following gates:

```text
parity gate:
  N even and q_s(j) odd  ->  x_s(N,j) odd

C3 gate (configuration-dependent):
  N != q_s(j) mod 3 at scanned indices  ->  3 does not divide x_s(N,j)

positivity gate:
  the scan is restricted to j with x_s(N,j) >= 5
```

Within the lapse prefix, the x side is automatically composite:

```text
established:
  j < tau_gs(N) and q_s(j) prime and x_s(N,j) >= 5
    ->  x_s(N,j) is composite
```

(x prime would give a Goldbach hit at j < tau_gs(N), contradicting
minimality.)  This is why x_prough_lapse splits into exactly
S_PR_lapse and A_PR_lapse with no prime case.

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

Write the lane signs as `q_a(t) = 6t + e_a` and `q_b(j) = 6j + e_b`
with `e_a, e_b in {+1, -1}`, and `B = N_b - N_a`.  The forced index is
defined explicitly:

```text
j_forced(t):
  t + (B + e_a - e_b) / 6
  defined iff 6 divides (B + e_a - e_b)

lane compatibility (established):
  j_forced is an integer  iff  B = e_b - e_a (mod 6)

alignment (established):
  if j_forced is defined:
    q_b(j_forced) = q_a(t) + B
    delta_x(j_forced,t) = 0
```

So `delta_x(j_forced,t) = 0` is a consequence of the definition of
`j_forced`, not an independent condition.  A g-sync event is:

```text
G_a(N_a,t)
valid_t(a,b,B,t)
6 divides (B + e_a - e_b), j_forced(t) >= 1
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


## Packed-index label convention

The packed index layer uses short labels for integer records.

```text
P:
  Prime(n)

S:
  Semiprime(n)

A:
  AlmostPrimeClass(n)

O:
  outside the arithmetic domain used by the index

U:
  unresolved stored label
```

`U` is not a mathematical class.  It is an explicit storage state.

For packed-index labels:

```text
S:
  Omega(n) = 2

A:
  Omega(n) >= 3
```

These labels do not change the definitions of tau, lapse, sync, or certificates.
