# Arithmetization corrections and additions

## Revision status (v0.1.2)

```text
applied in this revision:
  Part A corrections A1..A8   (docs/01, 02, 05, 07, 11)
  Part C items C1, C2          (README.ja.md, docs/04, CITATION.cff)
  Part C item C3               (clarifying comment in CITATION.cff)

recorded here, not inserted into docs/01..11:
  Part B additions B1..B6, except where an A-item required them
  (A5 domain gates and A6 lemmas are now in docs/01 and docs/02)

source of this review:
  external arithmetization review of package v0.1.1;
  finite validations in this file were run by the reviewer's
  harness, not by the included builder
```

## Status

```text
established:
  definition, direct consequence of definitions, or proof under stated
  preconditions

finite validation:
  checked in a stated finite range by the reviewer's harness

correction:
  the existing text is arithmetically inaccurate or weaker than provable

addition:
  new arithmetized statement proposed for the documentation set
```

This document reviews docs/01..11 for statements that can be arithmetized
and for arithmetic inaccuracies.  Each item names the file and section it
corrects or extends.  Nothing here claims an infinite theorem.

---

# Part A — Corrections

## A1. classify5 misroutes prime x <= P_def  [correction, established]

Target: docs/07 section 4 (classify5 flow), docs/05 section 5 (Shadow).

As written, classify5 tests the killed route before the prime route:

```text
2. If x has a small prime factor p with 5 <= p <= P_def:
     y = x / p
     if y is prime: return S_K
     else:          return A_K
3. If x is prime: return P
```

For a prime x with 5 <= x <= P_def, step 2 fires with p = x and y = 1.
Since 1 is not prime, classify5 returns A_K.

```text
counterexample:
  classify5(7) = A_K        (as written)
  correct:      P
```

Fix (either is sufficient):

```text
step 2 condition:
  exists prime p with 5 <= p <= P_def, p | x, and p < x

equivalently:
  y = x / p with y >= 2
```

With the fix:

```text
classify5(7)   = P    (step 2 skipped, step 3 fires)
classify5(25)  = S_K  (p=5 < 25, y=5 prime)        unchanged
classify5(125) = A_K  (p=5, y=25 composite)        unchanged
```

The same edge appears in the Shadow equivalence in docs/05 section 5:

```text
as written:
  Shadow(x) ... equivalently, x has a small factor p with 5 <= p <= P_def

corrected:
  Shadow(x):
    x is composite
    and exists prime p with 5 <= p <= P_def and p | x
```

For composite x every prime factor satisfies p < x, so the composite gate
makes the clause exact.  Note this edge is invisible in the intended domain
(x >> P_def); it matters only for the formal statement and for any future
use of classify5 on small inputs.  The included packed-index builder
(tau_c0c2cm_index/main.c) does not have this defect: its sieve marks only
multiples m >= p*p, so a prime x is never marked.

## A2. S(x) >= 1 already implies composite  [correction, established]

Target: docs/11 section 6 (Lemma 6.4, Theorem 6.5), docs/07
(S(x)+Exc(x) summary).

The current text hedges:

```text
S(x) = 1 and not Exc(x):
  Prime or Semi
  Semi under the composite precondition
```

The hedge is unnecessary.  If S(x) >= 1, some prime p satisfies p | x and
p*p <= x, hence 1 < p <= sqrt(x) < x, so p is a proper divisor and x is
composite.  Therefore:

```text
Lemma A2.1 [established]:
  S(x) >= 1  ->  x is composite

Theorem 6.5, strengthened branch [established]:
  Unit6(x) and S(x) = 1 and not Exc(x)  ->  Semi(x)
```

No composite precondition is needed on this branch.  The precondition
remains meaningful only in the implementation narrative (where S_total
arrives via a certified-composite path), not in the arithmetic statement.

Proof polish for Lemma 6.4: the case m = 1 in "m is 1 or prime" is
impossible under S(x)=1, because x = p would give S(x) = 0 (p*p <= p
fails for p >= 2).  Also state explicitly that gcd(m,6)=1 follows from
Unit6(x), so the second counted prime q >= 5.

## A3. tau totality convention  [correction, established]

Target: docs/01 (Lapse), docs/05 section 7, docs/11 section 16.1.

```text
tau_Cs(N) := min { j : C_s(N,j) holds }
```

is partial: if no j satisfies C, the minimum does not exist and the
relation `tau_Cs(N) > J iff lapse_Cs(N,J)` is ill-typed on the left.
Adopt the explicit convention:

```text
tau_Cs(N) := min { j >= 1 : C_s(N,j) }   if the set is nonempty
tau_Cs(N) := infinity                     otherwise

lapse_Cs(N,0) := true                     (empty prefix)
```

With this convention both stated relations hold for all J >= 0,
including the no-hit case, and LapseLen(N) = tau_g(N) - 1 is read as
infinity when tau_g(N) = infinity.  This matters for arithmetization
and for any Lean/Coq formalization, where min over a possibly empty
set must be total or explicitly partial.

## A4. tau_qs does not depend on N  [correction, established]

Target: docs/05 section 3.

```text
tau_qs(N) := min { j : Q_s(j) }
```

Q_s(j) contains no occurrence of N, so tau_qs is a constant function of
N.  Either drop the argument (write tau_qs) or add a note that the value
is N-independent.  Leaving the argument suggests a dependence that does
not exist.

## A5. Lapse trichotomy: domain gates and wording  [correction, established]

Target: docs/01 (Lapse decomposition), docs/02 (Lapse as arithmetic
no-hit prefix).

Three precision points.

1. docs/01 writes "x has a factor p with 5 <= p <= P_def" where docs/05
   and docs/07 write "prime p".  Inside the scan domain the two are
   equivalent (x odd and 3 not dividing x force the smallest prime
   factor of any such divisor to be >= 5), but for consistency write
   "prime p" in docs/01 as well.

2. The trichotomy

```text
q_fail  or  x_killed_lapse  or  x_prough_lapse
```

   is exhaustive and exclusive over the lapse prefix only under the
   following arithmetic gates, which should be stated where the
   decomposition is introduced:

```text
parity gate [established]:
  N even and q_s(j) odd  ->  x_s(N,j) odd

C3 gate [established, configuration-dependent]:
  N != q_s(j) mod 3 at scanned indices  ->  3 does not divide x_s(N,j)

positivity gate:
  the scan is restricted to j with x_s(N,j) >= 5
  (equivalently q_s(j) <= N - 5)
```

   Without the positivity gate, x = 1 satisfies the literal
   x_prough_lapse condition (no factor in [5,P_def]) while being
   neither prime nor composite, and x <= 0 is outside the domain.

3. Within the lapse prefix and with q_s(j) prime, x_s(N,j) is
   automatically composite (x prime would give a Goldbach hit at
   j < tau_g, contradicting minimality).  State this one-line lemma:

```text
Lemma A5.1 [established]:
  j < tau_gs(N) and Q_s(j) and x_s(N,j) >= 5
    ->  x_s(N,j) is composite
```

   This is the arithmetic reason x_prough_lapse splits into exactly
   S_PR_lapse and A_PR_lapse with no prime residue case.

## A6. g-sync: define j_forced, derive delta_x = 0  [correction, established]

Target: docs/01 (Sync), docs/02 (g-sync transfer), docs/05 section 8.

The g-sync event is currently listed as four independent conditions, but
two of them are derivable once j_forced is defined explicitly.  Write
the lane signs as q_a(t) = 6t + e_a and q_b(j) = 6j + e_b with
e_a, e_b in {+1, -1}, and B = N_b - N_a.

```text
Definition A6.1 (forced index):
  j_forced(t) := t + (B + e_a - e_b) / 6
  defined iff 6 divides (B + e_a - e_b)

Lemma A6.2 (lane compatibility) [established]:
  j_forced is integral  iff  B = e_b - e_a (mod 6)

Lemma A6.3 (alignment) [established]:
  if j_forced is defined, then
    q_b(j_forced) = q_a(t) + B
  and
    delta_x(j_forced, t) = 0
```

Proof of A6.3: q_b(j_forced) = 6t + B + e_a - e_b + e_b = q_a(t) + B;
then x_b(N_b, j_forced) = N_a + B - q_a(t) - B = x_a(N_a, t).

```text
Theorem A6.4 (g-sync, minimal premises) [established]:
  G_a(N_a, t)
  and 6 | (B + e_a - e_b)
  and j_forced(t) >= 1
  and Prime(q_a(t) + B)
    ->  G_b(N_b, j_forced(t))
        and tau_gb(N_b) <= j_forced(t)
```

`delta_x = 0` and `valid_t` need not be listed as separate premises:
delta_x = 0 is Lemma A6.3, and valid_t(a,b,B,t) is exactly
Prime(q_a(t)) and Prime(q_a(t)+B), the first conjunct of which is
already inside G_a.  The existing event list is not wrong, but it is
redundant; the minimal form above is the arithmetized version.

Consistency check against the validated stream pairs
[finite validation]:

```text
55 -> 15 and 51 -> 11:  e_a = -1, e_b = +1,  e_b - e_a = 2 (mod 6)
  listed B in {68, 74}:  68 = 74 = 2 (mod 6)   consistent

11 -> 51 and 15 -> 55:  e_a = +1, e_b = -1,  e_b - e_a = 4 (mod 6)
  listed B in {70, 76}:  70 = 76 = 4 (mod 6)   consistent
```

The orig-rule values (B = 68 or 74) both lie in the admissible residue
class, so the orig rule selects within the lane-compatible set, as
expected.

## A7. PrimePair count zero: exact statement  [correction, established]

Target: docs/11 section 3, Corollary 3.2.

The current caution ("x is prime, almost-prime class, or outside the
composite case") can be replaced by an exact equivalence once x = 1 is
excluded:

```text
For x > 1:
  |PrimePair(x)| = 0  <=>  Prime(x) or Almost(x)
```

Reason: x > 1 is prime or composite; composite x has |PrimePair| = 1
iff Semi(x) (Corollary 3.2), hence 0 iff Almost(x).

## A8. tau_g vs tau_gs naming  [correction, definition]

Target: docs/11 section 8.1 versus docs/01 and docs/05.

docs/11 defines the stream-joined first hit

```text
tau_g(N) := min { j : exists s, GoldbachHit(N,j,s) }
```

while docs/01 and docs/05 define per-stream tau_gs(N).  Both are useful;
state the relation once:

```text
tau_g(N) = min over s of tau_gs(N)
```

with the infinity convention of A3.

---

# Part B — Additions (new arithmetized statements)

## B1. x-side parity and C3 lemmas  [addition, established]

Currently implicit; state once in docs/01 or docs/11 section 8:

```text
Lemma B1.1:
  N even  ->  for all j, s:  x_s(N,j) is odd

Lemma B1.2 (already present as the C3 separation rule):
  3 | x_s(N,j)  <=>  N = q_s(j) (mod 3)
```

Together these justify restricting endpoint composite certificates to
prime divisors p >= 5 inside the scan, which is the precondition for
the C0/C2/CM mark and for the killed/P-rough split.

## B2. Residue-sieve offset formula  [addition, established]

The x-side killed positions are residue classes in j (docs/02 states
the congruence but not the closed form).  For prime p >= 5 and lane
sign e:

```text
x_s(N,j) = 0 (mod p)
  <=>  6j + e = N (mod p)
  <=>  j = (N - e) * inv6_p (mod p)

where inv6_p := 6^(-1) mod p   (exists since gcd(6,p) = 1)
```

Define j_p(N) := (N - e) * inv6_p mod p.  This is the explicit
arithmetization of the per-N sieve offset and shows directly why p = 2
and p = 3 are excluded from the residue sieve (inv6_p does not exist)
and must be handled structurally by parity and lane/C3 selection.

## B3. Mark type is determined by x mod 6  [addition, established; finite validation]

For p, y with p*y = x and p = 1 or 5 (mod 6), y = 1 or 5 (mod 6):

```text
Lemma B3.1:
  x = 1 (mod 6)  <=>  mark(p,y) in {C0, C2}
  x = 5 (mod 6)  <=>  mark(p,y) = CM
```

Proof: 1*1 = 1, 5*5 = 1, 1*5 = 5*1 = 5 (mod 6); all four cases.

Consequences:

```text
resmask(x) for x = 1 (mod 6):  subset of {C0, C2, PR}
resmask(x) for x = 5 (mod 6):  subset of {CM, PR}

S_CM(x) = 0  when x = 1 (mod 6)
S_C0(x) = S_C2(x) = 0  when x = 5 (mod 6)
```

Finite validation: checked for all Unit6 n <= 2,000,000 against a
reimplementation of the builder's marking loop; zero violations.
This is a cheap per-record invariant suitable for a future
`verify` command on index files.

## B4. Killed-route well-definedness  [addition, established]

classify5 step 2 and the S_K/A_K split do not depend on which small
prime factor is chosen:

```text
Lemma B4.1:
  x composite, p any prime divisor of x, y = x/p:
    y prime  <=>  Semi(x)
```

Reason: if y is prime then Omega(x) = 2; if Omega(x) = 2 then the
cofactor of any prime divisor is prime.  Hence "first" in
"first prime factor" is a performance choice, not a correctness
requirement, in both classify5 and the section-12 large-x classifier.

## B5. Evidence-relative endpoint state  [addition, established as definitions and lemmas]

This arithmetizes the design fact that endpoint generation precedes
verification (the endpoint exists before its prime status does).

```text
Certificates:

CompCert(x):
  c = (p, y) with Prime(p), p >= 5, y >= 2, p*y = x

PrimeCert(x):
  forall d (2 <= d and d*d <= x  ->  not (d | x))

Evidence set E:
  finite set of entries (x, c); an entry is admissible only if its
  certificate condition is checked at insertion time
  (for CompCert: re-multiply p*y = x; for PRIME entries: the
  PrimeCert procedure, e.g. MR64 under its stated width theorem)
```

The asymmetry is itself arithmetic: compositeness has a constant-size
existential witness (p, y); primality is a bounded universal statement
whose natural witness is the whole search.  This is the formal content
of "the endpoint occurs before prime".

```text
State(x, E):
  O          if not Unit6(x)
  P_CERT     if (x, PRIME) in E
  Ck_CERT    if exists (x,(p,y)) in E with mark(p,y) = Ck
  P_UNKNOWN  otherwise

Theorem B5.1 (birth state):
  Unit6(x)  ->  State(x, empty set) = P_UNKNOWN

Theorem B5.2 (monotone certification):
  E subset of E', E' sound  ->  certified states never revert
```

Evidence-relative tau:

```text
GoldbachHit(N,j,s,E):
  State(q_s(j),E) = P_CERT and State(x_s(N,j),E) = P_CERT

tau_g(N,E) := min { j : exists s, GoldbachHit(N,j,s,E) }
              (infinity convention of A3)

Theorem B5.3 (monotonicity):
  E subset of E'  ->  tau_g(N,E') <= tau_g(N,E)

Theorem B5.4 (soundness bound):
  E sound (no false certificates)  ->  tau_g(N,E) >= tau_g(N)

Theorem B5.5 (coverage gate):
  Complete(E,N,J) :=
    forall j <= J, forall s:
      State(q_s(j),E) != P_UNKNOWN and State(x_s(N,j),E) != P_UNKNOWN

  Complete(E,N,J) and tau_g(N) <= J  ->  tau_g(N,E) = tau_g(N)
```

Proof sketch of B5.5: let j* = tau_g(N) <= J; completeness gives both
endpoint states non-UNKNOWN at j*; soundness forces them to match the
true classes, hence both P_CERT; so tau_g(N,E) <= j*, and B5.4 gives
equality.

Under this formulation the safety invariants

```text
P_UNKNOWN_used_as_prime = 0
false_skip_j = 0
```

become consequences of (soundness + completeness up to the scanned
window) rather than free-standing conventions.  Note that B5.5 covers
the static evidence picture; the procedural rule "priority may reorder
but not delete" (docs/11 section 16.5) is the operational discipline
that preserves the completeness premise during a scan and is not
replaced by B5.5.

---

## B6. Proof-object translation of the verification pipeline  [addition]

This section grounds the meta-theorems about verification (here called
G1/G2: true-instance data adds no deductive power to a Sigma1-complete
theory; Pi1 statements are finitely refutable but not finitely provable)
in the repository's own certificates and theorems.  It depends on B5.

Status of this section:

```text
definitions:    the dictionary and proof-object formats
established:    the lemmas, given the dictionary
not claimed:    any infinite theorem; any minimal-strength claim for T
```

### B6.1 Background theory and registered lemmas

Fix a background theory T that is Sigma1-complete and proves elementary
number theory (PA suffices; the minimal sufficient fragment is left
open).  Register the repository's established results as T-lemmas:

```text
L1: Theorem 6.5 with A2        (S(x)+Exc classification, strengthened)
L2: Lemma B4.1                 (killed-route well-definedness)
L3: Lemma B3.1                 (mark type from x mod 6)
L4: Lemmas A6.2, A6.3, A6.4    (lane compatibility, alignment, g-sync)
L5: Lemma A5.1 + gates of A5   (lapse-side compositeness)
```

Each is elementary and provable in PA; how far below PA the proofs
descend is not claimed here.

### B6.2 Certificate dictionary (certificates to bounded sentences)

Each admissible evidence entry of B5 translates to a Delta0 sentence
(all quantifiers bounded):

```text
CompCert(x)=(p,y)      |->   p*y = x  and  5 <= p  and  2 <= y
                             and  (forall d <= sqrt(p)) (d >= 2 -> not d|p)

PrimeCert(x)           |->   (forall d <= sqrt(x)) (d >= 2 -> not d|x)

mark data              |->   p mod 6 = a  and  y mod 6 = b
                             (carried inside the CompCert sentence)

G_s(N,j)               |->   PrimeCert(q_s(j))  and  PrimeCert(N - q_s(j))

TwinCompanion(x)       |->   PrimeCert(x)  and  PrimeCert(x+2)

valid_t(a,b,B,t)       |->   PrimeCert(q_a(t))  and  PrimeCert(q_a(t)+B)
```

The state U has no translation.  U is the absence of an entry in E,
not a sentence; the safety rule "U must not be treated as P" is
enforced structurally: no inference rule introduces PrimeCert from the
absence of a certificate.  This is the formal content of
P_UNKNOWN_used_as_prime = 0 at the proof-object level.

MR64 note.  The soundness statement of deterministic 64-bit
Miller-Rabin,

```text
Ax_MR64:  forall x < 2^64:  MRpass(x, W) -> Prime(x)
```

is a bounded (Delta0) sentence.  If it is true, T proves it by
Sigma1-completeness, so it is not logically an extra axiom; but its
only known T-proof is astronomically long (it encodes the finite
pseudoprime enumeration).  Treating Ax_MR64 as a named lemma with an
external computational warrant is therefore a proof-length decision,
not a logical concession.  A formalization that wants to avoid the
warrant must replace MR64-based PrimeCert by the trial-division form
above, at higher computational cost.

### B6.3 Instance proof objects

Two different instance theorems must not be conflated.

```text
Existence instance:
  phi(N) :=  exists q,x <= N:  q + x = N  and  Prime(q)  and  Prime(x)

Exact-tau instance:
  tau_g(N) = j_star
```

A proof object for phi(N) is small:

```text
pi_phi(N):
  PrimeCert(q_s(j_star))
  PrimeCert(x_s(N, j_star))
  the identity q_s(j_star) + x_s(N, j_star) = N
```

A proof object for tau_g(N) = j_star additionally needs minimality,
and this is exactly where the lapse certificates live:

```text
pi_tau(N):
  pi_phi(N)
  plus, for every j < j_star and every scanned lane s:
    a CompCert for q_s(j),  or
    a CompCert for x_s(N,j)
  plus the domain gates of A5 (parity, C3 separation, positivity)
```

In B5 terms, pi_tau(N) is a witness of Complete(E, N, j_star - 1)
together with the hit at j_star; Theorem B5.5 is the schema that turns
this evidence into the exact-tau theorem.  The C0/C2/CM marks are the
residue bookkeeping of the CompCerts inside pi_tau; they add audit
structure, not deductive strength.

Worked micro-example (N = 96, chosen with N = 0 mod 3 so both lanes
are C3-free):

```text
j = 1, plus lane:   q = 7,  x = 89
j = 1, minus lane:  q = 5,  x = 91 = 7 * 13

pi_phi(96):
  PrimeCert(7), PrimeCert(89), 7 + 89 = 96

pi_tau(96) for tau_g(96) = 1 additionally records:
  CompCert(91) = (7, 13)   with mark C0   (7 = 1, 13 = 1 mod 6)
```

### B6.4 Sync as certificate reuse

Let pi_phi(N_a) be given, with its x-side certificate
PrimeCert(x_a(N_a, t)).  Suppose the premises of Theorem A6.4 hold:
lane compatibility 6 | (B + e_a - e_b), j_forced(t) >= 1, and
PrimeCert(q_a(t) + B).  Then by Lemma A6.3,

```text
x_b(N_b, j_forced) = x_a(N_a, t)
```

so the x-side certificate is reused verbatim, and

```text
pi_phi(N_b) :=
  PrimeCert(q_a(t) + B)          (new, small side)
  PrimeCert(x_a(N_a, t))         (reused, large side)
  the identity from A6.3
```

Accounting form (cost = multiset of certificates, with the large-x
certificate the dominant element since x is near N and q is near 6j):

```text
Lemma B6.4.1 [established]:
  a direct pi_phi(N_b) requires one small-q and one large-x
  certificate; a sync-derived pi_phi(N_b) requires one small-q
  certificate plus a reference to an existing large-x certificate.
```

This is the precise sense in which the documents' arithmetic
identities (delta_x = 0 via A6.3) act at the proof level: g-sync is
certificate reuse for the expensive side.  No claim is made that this
shortens proofs by any specific factor; B6.4.1 only states which
certificate is avoided.

### B6.5 The gap, restated inside the framework

```text
Proposition B6.5.1 [established]:
  Let Pi_X = { pi_phi(N) : N even, N <= X } be any family of instance
  proof objects produced by the pipeline, with every certificate
  admissible (B5).  Each pi_phi(N) is a T-proof, so its conclusion is
  already a T-theorem.  Hence Thm(T + conclusions(Pi_X)) = Thm(T), and
  in particular the family neither proves nor approaches
  (forall N) phi(N).  Sync changes the cost of producing Pi_X; it does
  not change this proposition.

Proposition B6.5.2 (refutation object) [established]:
  If phi(N0) is false for some even N0, the pipeline can in principle
  produce a finite refutation object: CompCerts or q-side CompCerts
  covering every j with x_s(N0,j) >= 5 on every lane (a complete lapse
  to the positivity bound j <= (N0-5)/6).  This object is a T-proof of
  not phi(N0), hence of the negation of the universal statement.
```

Together: the pipeline is an instance-theorem factory with a
refutation capability; the universal statement is outside its
deductive reach for the general reason above, now stated with the
repository's own certificate types.

### B6.6 Expressibility versus availability

The dictionary can express the P_def-split sentences:

```text
KILLED_Pdef(x)   |->  exists prime p:  5 <= p <= 193  and  p | x
                      (Delta0; the witness p is part of a certificate)
```

so S_K/S_PR/A_K/A_PR and the nearend taus are translatable in
principle.  What the current one-byte index lacks is not
expressibility but stored witnesses (docs/08, docs/09): the sentences
are writable, the certificate data to instantiate them is not
retained.  This separates the two limitations cleanly:

```text
dictionary limitation:   none found at the P/S/A and endpoint level
data limitation:         P_def split witnesses not stored (known)
```


# Part C — Non-arithmetic consistency notes


These are documentation-consistency findings from README.md, README.ja.md,
tau_c0c2cm_index/README.txt, and CITATION.cff.  No arithmetic errors were
found in these files; the formulas they restate (record layout, offset,
disk-size, Omega label meanings, resolved-label flow) all match docs/08
and main.c.

```text
C1. stale "planned" wording:
  README.ja.md ("予定command interface / コードはあとで追加します") and
  docs/04 ("Code will be added separately. The intended command path is")
  still describe the implementation as future work, while later sections
  of the same files state that tau_c0c2cm_index/ is included.  Update the
  early sections to present tense.

C2. version mismatch:
  CITATION.cff says version: 0.1.0; the package name says v0_1_1.
  Update CITATION.cff in the same commit as each release.

C3. license metadata:
  CITATION.cff says license: MIT only, while README declares dual
  licensing (docs CC BY 4.0, code MIT).  CITATION.cff describes the
  software, so MIT may be intentional, but a comment or the
  license-url field would remove the ambiguity.
```

# Items checked and found arithmetically correct

```text
docs/02: x = 0 (mod p) iff q = N (mod p)            identity, correct
docs/02: B = 74 pair form {6t-1, 6t+73}             correct
docs/03: class counts 110734+29151+32479 = 172364    exact
docs/03: percentage pairs sum to 100                 correct
docs/04: 10^11 + 64 bytes = 93.13 GiB                correct (93.132...)
docs/07: residual 285/1480 = 19.3 percent            correct (19.26)
docs/08: record bit layout vs main.c pack_record     consistent
docs/08: offset(n) = 64 + (n-1)                      matches code
docs/11: Lemmas 3.1, 6.2, 6.3, Prop 4.1, 4.2, 7.1    proofs correct
docs/11: x = 775 two-layer example                   correct
docs/11: section 12 large-x classifier and both
         square-power behaviors                      correct
         (m < cbrt(x) bound: p > cbrt(x) gives
          m = x/p^2 < x^(1/3))
```

# Not checked here

```text
finite validation summary values themselves (docs/03):
  source data not in the package; sums and ratios checked only
  for internal consistency

MR64 witness-set theorem:
  external dependency; treated as an axiom in B5

stream-name to lane-sign table beyond what docs/02 states:
  inferred from the 55 -> 15 example and the B residue check
```
