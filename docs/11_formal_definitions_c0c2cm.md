# Formal definitions for C0/C2/CM and S(x)+Exc(x)

## Status

```text
established:
  definition, direct consequence of definitions, or proof under stated
  preconditions

strong evidence:
  finite validation with stated range or sample conditions, but no global proof

conjecture:
  observed pattern without enough validation

unknown:
  not established by this repository
```

This document is a correction and consolidation document.  It separates the
objects that were previously easy to confuse:

```text
PrimePair layer:
  prime-pair definitions for semiprime numbers

DivPair layer:
  all-divisor diagnostic counts

SPD theorem layer:
  small-prime-divisor count S(x) and square-power exception Exc(x)

C0/C2/CM implementation layer:
  residue-separated implementation of the SPD theorem

Endpoint layer:
  endpoint state and scan safety

Packed-index layer:
  one-byte stored label and diagnostic mask
```

The current `tau_c0c2cm_index` package uses the packed-index layer.  It does
not store every theorem witness needed to reconstruct all diagnostic labels.

---

## 1. Base notation

```text
Z+:
  positive integers

Prime(n):
  n > 1 and the only positive divisors of n are 1 and n

Omega(n):
  number of prime factors of n counted with multiplicity

Omega(1):
  0

Omega(p):
  1 if Prime(p)

Omega(a*b):
  Omega(a) + Omega(b)
```

```text
Semi(n):
  Omega(n) = 2

Almost(n):
  n is composite and Omega(n) >= 3
```

`Almost` is a project-local class name in this repository.

```text
AlmostPrimeClass(n):
  n is composite and not semiprime
```

For this repository:

```text
Almost(n) <=> AlmostPrimeClass(n)
```

under the usual composite precondition.

---

## 2. Domain gates

Most C0/C2/CM residue statements require the 6k+/-1 domain.

```text
Unit6(n):
  n > 3 and gcd(n,6) = 1

equivalently:
  n > 3 and n == 1 or 5 (mod 6)
```

Do not state C0/C2/CM equivalences for all semiprimes without this gate.

Counterexample without the gate:

```text
10 = 2 * 5
Semi(10) is true
but 10 is not a product of two primes >= 5
```

Therefore, C0/C2/CM definitions that use mod-6 classes of both prime factors
must be scoped to `Unit6(n)` or an equivalent precondition.

---

## 3. PrimePair layer

```text
PrimePair(x):
  { (p,q) :
      Prime(p)
      Prime(q)
      p <= q
      p*q = x }
```

### Lemma 3.1  PrimePair uniqueness  [established]

```text
|PrimePair(x)| <= 1
```

Reason:

```text
p1*q1 = p2*q2 = x
```

with all four factors prime.  By unique factorization, the multisets are equal.
After normalization by `p <= q`, the pairs are equal.

### Corollary 3.2  Semiprime and PrimePair  [established]

For `x > 1`:

```text
Semi(x) <=> |PrimePair(x)| = 1
```

For `x > 1`:

```text
|PrimePair(x)| = 0  <=>  Prime(x) or Almost(x)
```

(established: `x > 1` is prime or composite, and composite `x` has
`|PrimePair(x)| = 1` iff `Semi(x)`.)

Do not state:

```text
Prime(x) or Almost(x) <=> |PrimePair(x)| = 0
```

without excluding `x=1` and other outside cases.

---

## 4. PrimePair C0/C2/CM layer

For primes `p >= 5`:

```text
M1(p):
  p == 1 (mod 6)

M5(p):
  p == 5 (mod 6)
```

Exactly one of `M1(p)` and `M5(p)` holds.

For `Unit6(x)`:

```text
C0_PP(x):
  exists (p,q) in PrimePair(x):
    M1(p) and M1(q)

C2_PP(x):
  exists (p,q) in PrimePair(x):
    M5(p) and M5(q)

CM_PP(x):
  exists (p,q) in PrimePair(x):
    (M1(p) and M5(q)) or (M5(p) and M1(q))
```

PrimePair repcounts:

```text
rC0_PP(x):
  |{ (p,q) in PrimePair(x) : M1(p) and M1(q) }|

rC2_PP(x):
  |{ (p,q) in PrimePair(x) : M5(p) and M5(q) }|

rCM_PP(x):
  |{ (p,q) in PrimePair(x) :
       (M1(p) and M5(q)) or (M5(p) and M1(q)) }|
```

Important correction:

```text
rCM_PP must not be defined as M1(p) != M5(q)
```

because that expression is true for an M1-M1 pair and would incorrectly count
C0 pairs as CM.

### Proposition 4.1  PrimePair residue split  [established]

For `Unit6(x)`:

```text
C0_PP(x) or C2_PP(x) or CM_PP(x)
  <=> Semi(x)
```

Reason:

```text
Semi(x) gives one prime pair (p,q).
Unit6(x) excludes factors 2 and 3.
Thus p,q >= 5 and each is M1 or M5.
```

The converse follows because each predicate requires a prime pair.

### Proposition 4.2  PrimePair exclusivity  [established]

For `Unit6(x)` and `Semi(x)`:

```text
exactly one of C0_PP(x), C2_PP(x), CM_PP(x) holds
```

### Warning 4.3  C0C2 overlap in PrimePair layer

In the PrimePair layer:

```text
rC0_PP(x) + rC2_PP(x) + rCM_PP(x) in {0,1}
```

Therefore:

```text
rC0_PP(x) > 0 and rC2_PP(x) > 0
```

is impossible.  A theorem of the form:

```text
C0C2_overlap_PP(x) -> Almost(x)
```

is vacuous in this layer.  Operational C0/C2 overlap must be discussed in the
DivPair or SPD implementation layer, not in the PrimePair layer.

---

## 5. DivPair diagnostic layer

This layer counts nontrivial divisor pairs.  Cofactors are not required to be
prime.

```text
DivPair(x):
  { d :
      2 <= d <= floor(sqrt(x))
      d divides x
      x/d >= 2 }
```

Residue-pair diagnostic sets:

```text
D_C0(x):
  { d in DivPair(x) :
      d == 1 (mod 6)
      and x/d == 1 (mod 6) }

D_C2(x):
  { d in DivPair(x) :
      d == 5 (mod 6)
      and x/d == 5 (mod 6) }

D_CM(x):
  { d in DivPair(x) :
      (d == 1 (mod 6) and x/d == 5 (mod 6))
      or
      (d == 5 (mod 6) and x/d == 1 (mod 6)) }
```

Counts:

```text
rC0_DP(x) := |D_C0(x)|
rC2_DP(x) := |D_C2(x)|
rCM_DP(x) := |D_CM(x)|
```

This layer is diagnostic.  It is not the safe theorem layer for semiprime vs
almost-prime classification.

### Corrected example: x = 775

```text
775 = 5 * 5 * 31
sqrt(775) ~= 27.8
```

All-divisor pairs counted by `DivPair` include:

```text
d = 5:
  5 * 155
  C2 type

d = 25:
  25 * 31
  C0 type
```

Therefore, under the all-divisor diagnostic layer:

```text
C2_only_DP(775):
  false

C0C2_overlap_DP(775):
  true
```

Do not use `775` as an all-divisor `C2_only -> Semi` counterexample.  It is a
counterexample only if the operational layer counts the small prime divisor
`p=5` but not the composite divisor `d=25`.

---

## 6. Small-prime-divisor theorem layer

This is the safe theorem route for classifying `Unit6(x)`.

```text
S(x):
  |{ p :
       Prime(p)
       p >= 5
       p*p <= x
       p divides x }|
```

`S(x)` counts distinct small prime divisors of `x`.  It is not an all-divisor
count and not a PrimePair count.

When `S(x)=1`, write the unique counted prime as:

```text
sp(x)
```

### Definition 6.1  Square-power exception

For `Unit6(x)` and `S(x)=1`:

```text
Exc(x):
  let p = sp(x)
  p*p divides x
  and x != p*p
```

This is the correct exception predicate.

The older phrase:

```text
prime cube exception
```

is only a subcase.  `Exc(x)` also covers:

```text
p^2*q
p^2*m
p^k for k >= 3
p^a*m for a >= 3
```

when the stated preconditions are satisfied.

### Lemma 6.2  No small prime divisor implies prime  [established]

For `Unit6(x)`:

```text
S(x) = 0 -> Prime(x)
```

Reason:

If `x` is composite, it has a prime factor `p <= sqrt(x)`.  Since `Unit6(x)`
excludes 2 and 3, that factor has `p >= 5` and is counted by `S(x)`, a
contradiction.

### Lemma 6.3  At least two small prime divisors implies Almost  [established]

For `Unit6(x)`:

```text
S(x) >= 2 -> Almost(x)
```

Reason:

Let `p1 < p2` be two distinct counted prime divisors.  Then `p1*p2` divides
`x`.  If `x = p1*p2`, then `p2 > sqrt(x)`, contradicting `p2*p2 <= x`.
Thus a remaining factor exists and `Omega(x) >= 3`.

### Lemma 6.4  One small prime divisor case  [established]

For `Unit6(x)`, `S(x)=1`, and `p=sp(x)`:

```text
if Exc(x):
  Almost(x)

if not Exc(x):
  Semi(x)
```

No composite precondition is needed:

```text
established:
  S(x) >= 1  ->  x is composite
```

because the counted prime `p` satisfies `1 < p <= sqrt(x) < x` and is
therefore a proper divisor of `x`.  In particular `Prime(x)` is
impossible under `S(x)=1`.

Reason for the Semi branch:

If `p` divides `x` only once, write `x=p*m`.  The case `m=1` would give
`x=p`, hence `S(x)=0` (`p*p <= p` fails), contradicting `S(x)=1`; so
`m >= 2`.  Since `Unit6(x)`, `gcd(m,6)=1`.  Any composite `m` would have
a prime factor `q <= sqrt(m) < sqrt(x)` with `q >= 5`, `q != p`, and
`q*q <= m <= x`, giving another counted prime divisor and contradicting
`S(x)=1`.  Thus `m` is prime and `x` is semiprime.  If `p*p` divides
`x`: either `x=p^2`, which is semiprime and not `Exc`, or `x != p^2`,
which is `Exc` and gives `Omega(x) >= 3`.

### Theorem 6.5  S(x)+Exc(x) classifier  [established]

For `Unit6(x)`:

```text
S(x) = 0:
  Prime

S(x) >= 2:
  Almost

S(x) = 1 and Exc(x):
  Almost

S(x) = 1 and not Exc(x):
  Semi
```

The branch needs no composite precondition: `S(x) >= 1` already implies
`x` is composite (Lemma 6.4).  The precondition remains meaningful only
in the implementation narrative, where `S_total` arrives via a
certified-composite path.

This is the safe theorem route.  C0/C2/CM residue labels are implementation and
diagnostic data for this theorem route.

---

## 7. Residue-separated implementation of S(x)

For `Unit6(x)`:

```text
S_C0(x):
  |{ p :
       Prime(p)
       p >= 5
       p*p <= x
       p divides x
       p == 1 (mod 6)
       x/p == 1 (mod 6) }|

S_C2(x):
  |{ p :
       Prime(p)
       p >= 5
       p*p <= x
       p divides x
       p == 5 (mod 6)
       x/p == 5 (mod 6) }|

S_CM(x):
  |{ p :
       Prime(p)
       p >= 5
       p*p <= x
       p divides x
       p mod 6 != (x/p) mod 6 }|
```

The cofactor `x/p` is not required to be prime.

### Proposition 7.1  Residue split identity  [established]

For `Unit6(x)`:

```text
S(x) = S_C0(x) + S_C2(x) + S_CM(x)
```

Reason:

Every prime `p >= 5` is either 1 or 5 modulo 6.  Since `x` is also 1 or 5
modulo 6, the cofactor `x/p` is 1 or 5 modulo 6.  Each counted prime divisor
falls into exactly one of C0, C2, or CM.

### Implementation classifier

For a certified composite `Unit6(x)`:

```text
S_total(x) := S_C0(x) + S_C2(x) + S_CM(x)

if S_total(x) >= 2:
  Almost

else if S_total(x) = 1 and Exc(x):
  Almost

else if S_total(x) = 1 and not Exc(x):
  Semi

else:
  impossible for a certified composite Unit6(x)
```

Important correction:

```text
Do not claim C2_only -> Semi as a global Layer 1 theorem.
Do not use a single residue class as the theorem.
Use S_total + Exc.
```

---

## 8. Operational derivation from tau endpoints

This section explains how the C0/C2/CM implementation layer arose from the
tau endpoint scan.

### 8.1 Endpoint scan objects

For a stream sign `s`:

```text
q_s(j):
  6j + 1 or 6j - 1

x_s(N,j):
  N - q_s(j)

EndpointPair(N,j,s):
  (q_s(j), x_s(N,j))
```

The Goldbach endpoint predicate is:

```text
GoldbachHit(N,j,s):
  q_s(j) is certified prime
  and x_s(N,j) is certified prime
```

The first verified endpoint hit is:

```text
tau_g(N):
  min { j : exists s, GoldbachHit(N,j,s) }

relation to the per-stream values of docs/01 and docs/05:
  tau_g(N) = min over s of tau_gs(N)
  with the infinity convention when no hit exists
```

Thus `tau_g` is not created by a residue pattern.  It is created only after a
verified endpoint hit.

### 8.2 Why endpoint composite marks were needed

Before `tau_g(N)`, the scan sees failed endpoint pairs.

For each active `(N,j,s)` before the first hit:

```text
q_s(j) failed
or
x_s(N,j) failed
or
both failed
```

To audit why a candidate failed, the implementation records endpoint states:

```text
P_CERT:
  endpoint is certified prime

P_UNKNOWN:
  endpoint is not covered well enough to use as prime

C0_CERT / C2_CERT / CM_CERT:
  endpoint has a certified composite route through a residue-separated mark

O:
  outside the endpoint domain or trivial non-endpoint case
```

These are endpoint scan states.  They are not the same as packed integer
labels.

### 8.3 How C0/C2/CM comes from an endpoint divisor

For an endpoint value `x = x_s(N,j)` in the `6k+/-1` domain, a composite
certificate may be obtained by finding a prime divisor `p >= 5`.

The endpoint relation is:

```text
x = p * y
```

where:

```text
y = x / p
```

Because `x` and `p` are both `1` or `5 (mod 6)`, the cofactor `y` is also
`1` or `5 (mod 6)`.

This gives a residue-separated mark:

```text
C0:
  p == 1 (mod 6) and y == 1 (mod 6)

C2:
  p == 5 (mod 6) and y == 5 (mod 6)

CM:
  p mod 6 != y mod 6
```

The cofactor `y` does not need to be prime.  Therefore this is not the
PrimePair layer.  It is the residue-separated implementation layer for the
small-prime-divisor theorem.

### 8.4 From endpoint marks to S_total

The endpoint marks are a way to compute or approximate the residue-separated
small-prime-divisor counts:

```text
S_C0(x)
S_C2(x)
S_CM(x)
```

The implementation total is:

```text
S_total(x) = S_C0(x) + S_C2(x) + S_CM(x)
```

The safe semi/almost theorem is not:

```text
C0 alone
C2 alone
CM alone
```

The safe theorem is:

```text
S_total(x) + Exc(x)
```

For a certified composite endpoint in the `6k+/-1` domain:

```text
S_total >= 2:
  Almost

S_total = 1 and Exc:
  Almost

S_total = 1 and not Exc:
  Semi
```

### 8.5 Why prime exclusion comes first

C0/C2/CM semi/almost hints are valid only after the endpoint is known
composite.

The safe order is:

```text
1. endpoint coverage gate
2. prime gate
3. composite endpoint certification
4. S_total + Exc classification
5. packed label or diagnostic output
```

Unsafe order:

```text
1. see no mark
2. treat endpoint as prime
```

This is forbidden.

Safety invariant:

```text
P_UNKNOWN_used_as_prime = 0
```

### 8.6 How this led to the packed index

The packed index was introduced to cache finite endpoint and integer-class
information in a compact reusable form.

The current packed record stores:

```text
stored_label:
  P/S/A/O/U

mask:
  C0/C2/CM/PR
```

This is enough for compact arithmetic lookup and route diagnostics, but it is
not the full endpoint proof object.

The packed record does not fully store:

```text
S_C0 count
S_C2 count
S_CM count
S_total
sp(x)
Exc witness
P_def=193 killed/P-rough split
full endpoint coverage certificate
```

Therefore, the packed index is a compact result/cache layer derived from the
endpoint scan and classifier structure.  It is not a complete replacement for
the endpoint audit layer.

### 8.7 Relation to lapse and nearend tau

The lapse sequence records endpoint failures before `tau_g`.

```text
LapseSeq(N):
  endpoint states for active (j,s) with 1 <= j < tau_g(N)
```

C0/C2/CM route marks are useful in lapse analysis:

```text
pair_CC
q_any
x_any
pair_anyany
long-tail lapse diagnostics
window priority
```

However, nearend tau objects such as:

```text
tau_shadow
tau_eclipse
tau_fury
S_PR_lapse
A_PR_lapse
```

require more than the compact one-byte record if they need the exact
`P_def=193` killed/P-rough boundary or the exact `S_total/Exc` witness.


## 9. Endpoint state and packed-index label are different

Endpoint layer states are scan and coverage states.

```text
EndpointState:
  P_CERT
  P_UNKNOWN
  C0_CERT
  C2_CERT
  C0C2_CERT
  CM_CERT
  O
```

Packed-index labels are stored integer labels.

```text
stored_label:
  build-time label stored in the one-byte record

resolved_label:
  query-time label after optional factor-2/factor-3 cofactor resolution
```

```text
PackedLabel:
  P
  S
  A
  O
  U
```

These are different layers.

```text
C0_CERT, C2_CERT, CM_CERT:
  composite endpoint marks or route marks

P/S/A:
  arithmetic class labels

P_UNKNOWN:
  endpoint coverage is incomplete

U:
  packed-index stored label is unresolved
```

Safety rule:

```text
P_UNKNOWN must not be used as prime.
U must not be treated as P, S, or A without query/certificate/fallback.
```

`C0C2_CERT` should not be read as a direct proof of `Almost` without specifying
which layer supplies the theorem witness.

---

## 10. Current tau_c0c2cm_index storage limits

The current packed record stores:

```text
stored label:
  P/S/A/O/U

mask:
  C0/C2/CM/PR
```

It does not fully store:

```text
S_C0(x)
S_C2(x)
S_CM(x)
S_total(x)
sp(x)
Exc(x) witness
P_def=193 killed/P-rough boundary
S_K/S_PR/A_K/A_PR
```

Therefore:

```text
current packed index:
  supports arithmetic P/S/A lookup after query resolution

current packed index alone:
  does not fully reconstruct the theorem witness or nearend diagnostic split
```

If those values are needed later, use one of:

```text
sidecar data
extra packed bits/classes
recomputation during audit
trusted certificate table
```

---

## 11. P_def=193 killed/P-rough layer

The historical classifier uses:

```text
P_def = 193
```

Definitions:

```text
KILLED_Pdef(x):
  exists prime p with 5 <= p <= 193 and p divides x

P_ROUGH_Pdef(x):
  no prime p with 5 <= p <= 193 divides x
```

Historical nearend labels:

```text
S_K:
  Semi(x) and KILLED_Pdef(x)

S_PR:
  Semi(x) and P_ROUGH_Pdef(x)

A_K:
  Almost(x) and KILLED_Pdef(x)

A_PR:
  Almost(x) and P_ROUGH_Pdef(x)
```

Important distinction:

```text
RES_PR:
  packed-index route marker

P_ROUGH_Pdef(x):
  predicate using the exact P_def=193 boundary
```

Therefore:

```text
RES_PR != P_ROUGH_Pdef(x)
```

unless the implementation explicitly computes and stores that boundary.

Nearend objects such as:

```text
tau_shadow
tau_eclipse
tau_fury
S_PR_lapse
A_PR_lapse
```

need the killed/P-rough boundary if they are reconstructed after index build.

---

## 12. Large-x operational classifier

The theorem definition `S(x)` counts prime divisors up to `sqrt(x)`.  The
large-x operational classifier does not compute `S(x)` directly.

For large `x`, use:

```text
MR64(x)
prime scan up to floor_cbrt(x)
MR64(x/f)
```

Operational classifier for `Unit6(x)`:

```text
if MR64(x) says prime:
  Prime

f = first prime factor of x with f <= floor_cbrt(x)

if no such f:
  Semi

y = x / f

if MR64(y) says prime:
  Semi

else:
  Almost
```

Preconditions:

```text
MR64:
  deterministic for the stated integer width

prime scan:
  complete for primes <= floor_cbrt(x)

overflow:
  modular multiplication and p*p/p*p*p comparisons are safe
```

This is operationally equivalent to the `S(x)+Exc(x)` classification under
those preconditions, but it is not the same computation as directly evaluating
`S(x)`.

Pure-square behavior:

```text
x = p^2 and p > cbrt(x):
  cbrt scan finds no factor
  MR64(x) says composite
  classifier returns Semi
```

Non-square square-power behavior:

```text
x = p^2*m, m >= 2, p > cbrt(x):
  some prime factor of m is < cbrt(x)
  cbrt scan finds it
  cofactor remains composite
  classifier returns Almost
```

---

## 13. Validation and reproducibility requirements

Do not record a finite check as a global theorem.

A validation claim should state:

```text
statement:
  what was checked

layer:
  PrimePair / DivPair / SPD / packed-index / endpoint

range:
  exact finite range

sample:
  sample count and generator, if not exhaustive

seed:
  deterministic seed, if random sampling was used

command:
  exact command

code version:
  tag, commit, or source hash

output:
  summary and, if possible, output hash
```

Status rule:

```text
definition or proof under stated preconditions:
  established

exhaustive finite check:
  established only for that finite range

random or sampled check:
  strong evidence

unbounded claim without proof:
  unknown or conjecture
```

---

## 14. What cannot be claimed

Do not claim:

```text
C2_only -> Semi
```

without specifying the PrimePair layer and the `Unit6` domain.

Do not claim:

```text
C0C2_overlap -> Almost
```

from the PrimePair layer.  In that layer the overlap is impossible.

Do not claim:

```text
RES_PR = P_ROUGH_Pdef
```

unless the implementation explicitly computes and stores the `P_def=193`
boundary.

Do not claim that the current one-byte record stores:

```text
S_total
sp(x)
Exc(x)
S_K/S_PR/A_K/A_PR
```

It does not.

Do not claim that any finite validation proves an infinite theorem.

---

## 15. Summary

```text
safe theorem:
  S(x) + Exc(x)

implementation split:
  S_C0(x), S_C2(x), S_CM(x)
  S_total = S_C0 + S_C2 + S_CM

diagnostics:
  C0/C2/CM endpoint state
  overlap
  lapse
  pair features

packed index:
  P/S/A/O/U + C0/C2/CM/PR mask

not stored in current packed index:
  exact S counts
  Exc witness
  P_def=193 killed/P-rough split
```

This is the intended formal structure for the current documentation set.


## 16. Lapse, nearend tau, and diagnostic feature safety

This section records how lapse and related diagnostic features fit into the
current formal structure.

### 16.1 Lapse depends on tau_g

For an even target `N`, if `tau_g(N)` is defined:

```text
tau_g(N):
  min { j : exists s, GoldbachHit(N,j,s) }
```

then the lapse prefix is:

```text
LapseSeq(N):
  endpoint states for active (j,s) with 1 <= j < tau_g(N)

LapseLen(N):
  tau_g(N) - 1
```

Therefore:

```text
lapse is downstream of tau_g
```

It is not an independent proof of a Goldbach endpoint.

### 16.2 Lapse sequence record

Each lapse row should be read as an endpoint-pair diagnostic row:

```text
PairState(N,j,s):
  ( EndpointState(q_s(j)), EndpointState(x_s(N,j)) )
```

Possible row-level derived features:

```text
PairCC(N,j,s):
  q_s(j) is composite endpoint
  and x_s(N,j) is composite endpoint

QAnyOverlap(N,j,s):
  q-side endpoint has a C0/C2/CM overlap or count-overlap diagnostic

XAnyOverlap(N,j,s):
  x-side endpoint has a C0/C2/CM overlap or count-overlap diagnostic

PairAnyAny(N,j,s):
  QAnyOverlap(N,j,s) and XAnyOverlap(N,j,s)
```

These are diagnostic features.  They are not safe deletion rules by themselves.

### 16.3 Diagnostic features are not skip proofs

The following features may be used for analysis, ranking, or explanation:

```text
PairCC
QAnyOverlap
XAnyOverlap
PairAnyAny
C0/C2/CM route marks
C0C2-style overlap
long-tail lapse buckets
window priority score
```

They must not be used to delete a candidate `j` unless a separate certificate
proves that the deleted candidate cannot be the first Goldbach endpoint.

Safe usage:

```text
allowed:
  record feature
  rank feature
  prioritize window
  explain long-tail tendency
  decide what to audit first

not allowed without certificate:
  skip a candidate
  delete a candidate
  treat P_UNKNOWN as prime
  treat U as P/S/A
```

Safety invariant:

```text
false_skip_j = 0
P_UNKNOWN_used_as_prime = 0
```

### 16.4 Nearend tau objects require more data

Nearend objects include:

```text
tau_shadow
tau_eclipse
tau_fury
S_PR_lapse
A_PR_lapse
```

These objects are not fully determined by the current one-byte packed record.

Reason:

```text
current record stores:
  P/S/A/O/U
  C0/C2/CM/PR mask

current record does not fully store:
  exact S_C0/S_C2/S_CM counts
  S_total
  sp(x)
  Exc witness
  P_def=193 killed/P-rough boundary
  full endpoint coverage certificate
```

Therefore, reconstruction of nearend tau objects requires one of:

```text
sidecar data
extra packed bits/classes
recomputation during audit
trusted certificate table
```

### 16.5 Window priority rule

A priority score may reorder candidate checks inside a finite window.

```text
WindowPriorityScore(N,j,s):
  function of endpoint state, lapse features, overlap features, and j
```

Safe priority rule:

```text
priority may reorder
priority may not delete
tie-break by increasing j
```

Required invariant:

```text
the first certified GoldbachHit must remain observable
```

Thus window priority is a performance and diagnostics tool, not a theorem.

### 16.6 No-left-behind rule for lapse and nearend data

If a result depends on a tau, endpoint state, C0/C2/CM component, certificate,
or nearend diagnostic, then that predecessor must be one of:

```text
1. computed in an earlier lane,
2. recovered from cache or certificate,
3. explicitly marked unresolved or unaudited.
```

This applies to:

```text
lapse rows
nearend tau rows
pair overlap summaries
window priority summaries
packed-index query results
```

Do not silently promote an unresolved predecessor into a certified result.

### 16.7 Current package status

For `tau_c0c2cm_index`:

```text
active:
  packed full-index builder
  query-time stored_label/resolved_label distinction
  C0/C2/CM/PR compact mask

diagnostic but not fully stored:
  lapse feature witnesses
  exact S_total
  Exc witness
  P_def=193 killed/P-rough split
  nearend tau reconstruction inputs
```

Therefore:

```text
current package:
  can document and cache compact labels/routes

current package alone:
  is not a full lapse/nearend tau proof archive
```


## 17. Findings captured in this update

This update records the following design findings.

```text
1.
  C0/C2/CM is not the theorem itself.

2.
  The safe theorem route is S(x)+Exc(x).

3.
  C0/C2/CM is the residue-separated implementation and diagnostic layer.

4.
  The current packed index stores masks, not full S_C0/S_C2/S_CM counts.

5.
  RES_PR is a route marker, not P_ROUGH_Pdef.

6.
  S_K/S_PR/A_K/A_PR cannot be reconstructed from the current one-byte record
  alone.

7.
  EndpointState and packed labels are separate layers.

8.
  P_UNKNOWN and U are both non-use-as-prime states without further resolution.

9.
  Large-x fallback is MR64 + cbrt prime scan + cofactor MR64.

10.
  Validation claims must include range, generator or seed, command, and version.

11.
  Lapse/nearend features are diagnostic unless their predecessors are certified.

12.
  Window priority may reorder candidates but must not delete candidates without
  a certificate.
```
