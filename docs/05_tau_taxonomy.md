# Tau taxonomy and exact labels

This document lists the tau-related objects used by the repository.
It is intended to prevent confusion between residue filters, verified tau values, class labels, lapse values, and certificates.

Status convention:

```text
tau:
  verified first-hit time only

not tau:
  residue candidates
  cache entries
  class labels
  certificates
```

## 1. Stream values

For a stream `s` and index `j`:

```text
q_s(j):
  prime-side stream value

x_s(N,j):
  N - q_s(j)
```

The checked stream family uses:

```text
q_s(j) = 6j + 1
or
q_s(j) = 6j - 1
```

This `mod 6` layer is structural. It is not a tau.

## 2. Verified prime-side predicates

These are predicates, not necessarily stored columns.

```text
Q_s(j):
  q_s(j) is a prime number

X_s(N,j):
  x_s(N,j) is a prime number

G_s(N,j):
  Q_s(j) and X_s(N,j)
```

Notes:

```text
Q_s(j):
  q-side primality fact

X_s(N,j):
  x-side primality fact

G_s(N,j):
  Goldbach endpoint fact
```

`X_s` may be implemented through direct primality testing, complete class complement, or a verified certificate. It is not a candidate tau.

## 3. Primary tau values

General form:

```text
tau_Cs(N):
  min { j >= 1 : C_s(N,j) holds }   if such j exists
  infinity                           otherwise
```

The infinity convention makes tau total (see docs/01).

The main tau values are:

```text
tau_qs:
  min { j : Q_s(j) }

  first verified q-prime hit in stream s
  note: Q_s(j) does not involve N, so this value is N-independent;
  it is written without the N argument
```

```text
tau_xs(N):
  min { j : X_s(N,j) }

  first verified x-prime hit in stream s
  often called tau_x_ne_raw in development notes
```

```text
tau_gs(N):
  min { j : G_s(N,j) }

  first verified Goldbach endpoint hit in stream s
```

## 4. Nearend tau values

Nearend tau values require `q_s(j)` to be prime and classify `x_s(N,j)` as a non-endpoint component.

```text
tau_semis(N):
  min { j : Q_s(j) and Semiprime(x_s(N,j)) }
```

```text
tau_furys(N):
  min { j : Q_s(j) and Fury(x_s(N,j)) }
```

```text
tau_shadows(N):
  min { j : Q_s(j) and Shadow(x_s(N,j)) }
```

```text
tau_eclipses(N):
  min { j : Q_s(j) and Eclipse(x_s(N,j)) }
```

```text
tau_c3s(N):
  min { j : Q_s(j) and C3(x_s(N,j)) }
```

In the checked scan setting, `tau_c3s` is inactive because the residue
separation condition `N != q_s(j) mod 3` prevents C3 from occurring inside the
scan. C3 remains useful for companion classification outside that scan setting.

## 5. Project-local class predicates

```text
Semiprime(x):
  x = p*q
  p and q are prime
  p may equal q
```

```text
AlmostPrimeClass(x):
  x is composite
  and x is not semiprime
```

```text
C3(x):
  x > 3
  and x mod 3 = 0
```

```text
Fury(x):
  AlmostPrimeClass(x)
  and not C3(x)
```

```text
Shadow(x):
  x is a killed composite in the project classifier
  equivalently:
    x is composite
    and x has a prime factor p with 5 <= p <= P_def

  the composite gate matters at the small edge: a prime x with
  5 <= x <= P_def has the factor p = x, but it is not a killed
  composite; for composite x every prime factor satisfies p < x,
  so the gated clause is exact
```

```text
Eclipse(x):
  killed almost-prime side in the project classifier
  equivalently, a killed non-semiprime composite component
```

The names `Shadow`, `Eclipse`, and `Fury` are project-local diagnostic labels.
They are not standard number-theory names.

## 6. Roughness split labels

The classifier uses a small-prime bound `P_def`.

```text
KILLED(x):
  exists prime p with 5 <= p <= P_def and p divides x
```

```text
P_ROUGH(x):
  no prime p with 5 <= p <= P_def divides x
```

The main split labels are:

```text
S_PR:
  P-rough semiprime

S_K:
  killed semiprime

A_PR:
  P-rough almost-prime class

A_K:
  killed almost-prime class
```

These are class labels, not tau values. A tau can point to the first index at which such a label appears.

## 7. Lapse values

A lapse is the no-hit prefix associated with a tau.

```text
lapse_Cs(N,J):
  for all j <= J, C_s(N,j) does not hold
```

Relations:

```text
tau_Cs(N) > J
  iff
lapse_Cs(N,J)
```

```text
tau_Cs(N) = j_star
  iff
lapse_Cs(N,j_star-1)
and
C_s(N,j_star) holds
```

For Goldbach endpoints, prefix reasons are decomposed as:

```text
q_fail_s(j):
  q_s(j) is composite

x_killed_lapse_s(N,j):
  q_s(j) is prime
  and x_s(N,j) is killed by a small prime

x_prough_lapse_s(N,j):
  q_s(j) is prime
  and x_s(N,j) is P-rough
```

The checked setting further split `x_prough_lapse` into:

```text
S_PR_lapse
A_PR_lapse
```

## 8. Sync values

For source stream `a` and target stream `b`:

```text
delta_x(j,t):
  x_b(N_b,j) - x_a(N_a,t)
```

A `g-sync` fact uses:

```text
G_a(N_a,t)
valid_t(a,b,B,t)
6 divides (B + e_a - e_b), j_forced(t) >= 1
Prime(q_b(j_forced))
```

where `j_forced(t) = t + (B + e_a - e_b)/6` with lane signs
`e_a, e_b in {+1,-1}`; the equality `delta_x(j_forced,t) = 0` is then
a consequence of this definition, not an independent condition
(see docs/01 and docs/02 for the alignment lemma).

Then:

```text
G_b(N_b,j_forced)
tau_gb(N_b) <= j_forced
```

Component sync uses the same transfer idea for non-endpoint components.
For non-endpoint components, the repository distinguishes:

```text
latent C-sync:
  arithmetic transfer places component C at j_forced

observed C-sync:
  the scan reaches j_forced and records C
```

## 9. valid_t

For fixed source, target, and B:

```text
V_B(a,b) = { t :
  Prime(q_a(t))
  and Prime(q_a(t) + B)
}
```

`valid_t` is a verified prime-pair condition. It is not merely a residue condition.

## 10. Companion certificates

Companion certificates are not tau values. They are reusable facts attached to a value.

```text
TWIN companion certificate:
  Prime(x)
  and Prime(x+2)
```

The TWIN certificate lets certificate-mode validation decide twin-prime status without re-testing `x+2`.

## 11. Legacy names

Some development notes used names that are no longer primitive in the final documentation.

```text
gc:
  legacy name for g-sync

clone:
  legacy discovery label for shifted source-side events
  not a primitive object in the final tau-sync sieve definition

candidate tau:
  not used
  residue candidates are handled by the mod 6 layer, not by tau
```

## 12. Recommended naming in new files

Use:

```text
mod6_layer
verified_tau
g_sync
lapse_Cs
valid_t
TwinCompanion
```

Avoid:

```text
candidate_tau
prime_clone_tau
gc as a primitive
XPrime candidate tau
```


## 13. Classifier flow and parameters

For the exact classifier flow, default values `JMAX_DEFAULT=5000` and `P_def=193`, killed cofactor residual, and audit-output requirements, see:

```text
docs/07_classifier_and_parameters.md
```


## 14. Packed-index labels are not tau

```text
P/S/A/O/U:
  integer labels stored or displayed by the packed index

tau:
  verified first-hit time
```

A packed index record may support a tau computation, but the record itself is
not a tau value.
