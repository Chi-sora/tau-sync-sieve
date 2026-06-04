# 10_Experiment Report

## Overview

This report summarizes the tau-sync candidate-generation experiment.

The purpose of the experiment was to test whether tau-sync style filtering can reduce the number of twin-prime candidates before final verification.

The main result was that block-sieve based candidate generation was much faster than the earlier per-step residue update method.

---

## Definitions

### Twin Candidate

For an integer k:

```text
p0 = 6k - 1
p1 = 6k + 1
```

A twin-prime candidate is:

```text
TwinCandidate(k) = (6k - 1, 6k + 1)
```

### Residue Rejection Rule

For a small prime r greater than 3:

```text
6k - 1 == 0 mod r
or
6k + 1 == 0 mod r
```

means that at least one side of the pair is composite.

Because gcd(6,r)=1, the inverse of 6 modulo r exists.

Define:

```text
bad_minus = inv6 mod r
bad_plus  = -inv6 mod r
```

Then k is rejected when:

```text
k == bad_minus mod r
or
k == bad_plus mod r
```

---

## Initial Generator

The first generator used the following approach:

```text
for each k:
    update residue state
    test residue state
    emit candidate if not rejected
```

This worked correctly, but it still performed too many repeated residue updates.

---

## Candidate Generation Result

An observed run reached:

```text
step          : about 100,000,000
k_digits      : 9
candidate file: about 39 MB
candidates    : 2,203,806
```

The filter removed most states before final verification.

Approximate candidate rate:

```text
2,203,806 / 102,000,000
about 2.16 percent
```

---

## Verification Result

Example verified twin-prime pair:

```text
(104801, 104803)
```

Largest verified pair observed in the checked range:

```text
(611999681, 611999683)
```

---

## Performance Investigation

### Output Cost

The earlier implementation opened and closed the candidate CSV file for every emitted candidate.

This was changed to:

```text
open output file once
use a large write buffer
flush periodically
```

This improved performance, but it did not remove the main cost.

---

## Residue Evaluation Order

Two residue orders were compared:

```text
small-prime-first
large-prime-first
```

The small-prime-first order was much faster because small primes reject candidates more frequently.

Conclusion:

```text
evaluate high-rejection residues first
```

---

## Block Sieve Addition

### Motivation

The optimized generator still updated every residue at every step.

Conceptually:

```text
for each k:
    update all residues
    test all residues
```

This was unnecessary.

### New Idea

The block-sieve version processes a whole block at once.

Conceptually:

```text
for each block:
    mark invalid offsets using each small prime
    emit only unmarked offsets
    update base residues once
```

This removes per-step residue maintenance.

### Correctness Test

The block-sieve implementation was compared with the previous optimized generator.

Test condition:

```text
steps          = 100000
filter_primes  = 10000
```

Comparison items:

```text
candidate count
candidate order
candidate sequence
```

Result:

```text
candidate sequence matched exactly
```

Thus the block-sieve version changed the execution method, not the candidate definition.

### Performance Result

Observed benchmark:

```text
processed=10000000000
k_digits=11
emitted=159845476
filtered=9840154524
rate_steps_per_sec=62927513.80
```

Compared with the previous optimized version, the observed speed improvement was approximately:

```text
about 230x
```

---

## Interpretation

The main bottleneck was not the mathematical candidate definition itself.

The main bottleneck was redundant residue maintenance.

The block-sieve method reduced this redundant work.

---

## Practical Meaning

The experiment suggests that tau-sync style filtering is useful as:

```text
candidate compression
search pre-processing
final-verification reduction
```

It is not presented here as a direct proof method.

It is best understood as a high-speed candidate generator.

---

## Future Work

Possible next steps:

```text
1. Store candidates in binary format instead of CSV.
2. Save base_k plus offsets instead of full decimal k values.
3. Add a streaming final verifier.
4. Add block-level parallelization.
5. Measure rejection contribution per residue.
6. Compare several block sizes.
```

---

## Conclusion

The experiment demonstrated that:

```text
tau-sync style filtering can remove most candidates early
block-sieve marking preserves candidate correctness
redundant residue updates can be removed
candidate generation can exceed 60 million steps per second
```

The most important result is that the block-sieve approach preserves the same candidate set while greatly improving generation speed.
