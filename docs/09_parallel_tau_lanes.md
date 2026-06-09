# Parallel Tau Lanes for the tau-sync sieve

Author: Chisora

## Status

[established] This document defines a processing strategy for the tau-sync sieve.

[established] The goal is to avoid a monolithic scan that waits for every tau component before progressing.

[strong evidence] The method is useful when many tau components exist and branch/cache/IO waiting becomes a bottleneck.

[established] This is an execution strategy, not a change to the mathematical definitions of tau, lapse, or sync.

---

## 1. Problem

[established] A monolithic tau scan evaluates many components at the same index `j`.

```text
for j = 1..JMAX:
  check q
  check x
  check g
  check semiprime
  check shadow
  check eclipse
  check fury
  check c3
  update all tau fields
```

[strong evidence] This is safe, but it can become slow when the number of tau branches increases.

```text
main costs:
  branch-heavy control flow
  shared cache lookup
  global counter updates
  CSV/log output
  waiting for unrelated tau components
```

[established] The proposed alternative is to split the work into independent tau lanes.

---

## 2. Definition: tau lane

[established] A tau lane is an execution unit that is responsible for one tau component or one closely related group of components.

```text
lane_q:
  computes or looks up QPrime-related facts

lane_g:
  computes tau_g or verifies endpoint facts

lane_lapse:
  decomposes the prefix j < tau_g

lane_nearend:
  audits semiprime, shadow, eclipse, fury, c3 facts

lane_sync:
  verifies source -> target sync facts

lane_twin:
  verifies TWIN companion certificates
```

[established] A lane may run independently if all of its required predecessor facts are available.

---

## 3. Dependency graph

[established] The dependency graph prevents a lane from using a fact that has not been produced.

Let

```text
D = (V,E)
```

where

```text
V:
  predicates, tau values, lapse values, sync facts, tables, certificates

E:
  directed dependency edges
```

The edge

```text
A -> B
```

means

```text
B requires A.
```

[established] A lane may use node `B` only when all required predecessor nodes of `B` are known, cached, or explicitly marked as unaudited.

---

## 4. Core dependencies

[established] The endpoint lane depends on verified prime predicates.

```text
QPrime_s(j)  \
              -> G_s(N,j) -> tau_gs(N)
PrimeX_s(N,j)/
```

[established] Lapse depends on tau.

```text
tau_gs(N)=t
  -> lapse_gs(N,t-1)
```

[established] Sync depends on source endpoint, valid_t, target q, and delta_x.

```text
G_source(t)
valid_t(source,target,B,t)
delta_x(j_forced,t)=0
QPrime_target(j_forced)
  -> g_sync(source,target,t,j_forced)
  -> G_target(j_forced)
  -> tau_g_target <= j_forced
```

[established] TWIN certificate depends on a verified prime value and its companion.

```text
TWIN(x):
  Prime(x)
  AND Prime(x+2)
```

---

## 5. No-left-behind rule

[established] The no-left-behind rule prevents a fast lane from losing information needed by a later lane.

```text
No-left-behind rule:

If a result depends on a tau/component/certificate,
then that predecessor must be one of:

  1. computed in an earlier lane,
  2. recovered from cache/certificate,
  3. explicitly marked as unaudited.
```

[strong evidence] This rule is necessary when endpoint processing is separated from nearend processing.

Example:

```text
If lane_g finds tau_g early,
lane_nearend must not assume that later nearend facts were observed.

It must either:
  run an audit lane,
  use a certificate,
  or mark nearend fields as unaudited.
```

---

## 6. Endpoint-late compatibility

[established] For nearend observation, endpoint-first termination can hide nearend facts.

Therefore:

```text
scan_g:
  may stop at the endpoint

scan_tv:
  must record nearend facts before applying endpoint stop

scan_sync_audit:
  must continue until required j_forced positions are reached
```

[established] In the parallel-lane model, this becomes:

```text
lane_g:
  may produce endpoint bounds quickly

lane_nearend:
  is invoked only when observed nearend facts are required

lane_sync:
  may use endpoint bounds without forcing a full nearend scan
```

---

## 7. Recommended execution pipeline

[strong evidence] The safe high-speed pipeline is:

```text
1. mod6 layer
   q_s(j)=6j +/- 1
   C3 scan exclusion
   B and j_forced alignment

2. precompute lane
   q table
   q prime table
   valid_t table

3. endpoint lane
   tau_g or endpoint certificate

4. lapse lane
   q_fail
   x_killed_lapse
   x_prough_lapse
   S_PR_lapse
   A_PR_lapse

5. sync lane
   valid_t
   delta_x=0
   target tau_g bound

6. nearend audit lane
   semiprime
   shadow
   eclipse
   fury
   c3
   only when observed nearend evidence is requested

7. twin lane
   TWIN companion certificate
```

[established] The default validation path may skip the nearend audit lane when only g-sync and endpoint bounds are being validated.

---

## 8. Why this can be faster

[strong evidence] The parallel-lane method can be faster because it avoids making every tau component wait for every other tau component.

```text
monolithic scan:
  all tau components are checked together

parallel tau lanes:
  endpoint, lapse, sync, nearend, and twin are separated
```

Potential speed gains:

```text
1. fewer branches in each loop
2. fewer unrelated cache lookups
3. thread-local summaries
4. less lock contention
5. base-level parallelism
6. targeted nearend audits only when needed
```

[speculative] The largest speedup is expected from base-parallel execution and lane-specific work reduction, not from the lane split alone.

---

## 9. Correctness condition

[established] The lane split is correct only if the dependency graph is respected.

For every reported fact `F`:

```text
F is valid
  iff
all required predecessors of F are known
or F is explicitly marked as unaudited.
```

Examples:

```text
tau_g is valid:
  QPrime and PrimeX are verified or certified.

lapse_g is valid:
  tau_g is known.

g-sync is valid:
  source G, valid_t, delta_x=0, and target QPrime are known.

TWIN is valid:
  Prime(x) and Prime(x+2) are known or certified.

nearend observed sync is valid:
  the scan/audit lane reached j_forced and recorded the component.
```

---

## 10. What this method does not claim

[established] This method does not remove the need to create verified facts in raw build mode.

```text
raw build mode:
  creates verified tau/class/certificate facts

certificate mode:
  reuses verified facts

parallel tau lanes:
  make the processing order faster and safer,
  but do not create prime truth from nothing.
```

[established] `mod 6` is a structural residue layer, not a tau.

[established] Candidate tau is not used.

---

## 11. Summary

[strong evidence] The method can be summarized as:

```text
Do not make every tau wait for every other tau.

Split tau processing into lanes.

Use the dependency graph and no-left-behind rule to prevent missing required facts.
```

Recommended name:

```text
parallel tau lanes
```

or

```text
tau lane pipeline
```


## 12. Relation to packed-index builder

The packed index builder uses a simplified version of the same dependency
discipline.

```text
build-time unresolved:
  stored as U

cofactor route:
  marked with PR when applicable

query-time recovery:
  allowed to resolve U by reading one cofactor record
```

This is an implementation-level use of the no-left-behind rule.

```text
do not use U as P
do not use U as S
do not use U as A
```
