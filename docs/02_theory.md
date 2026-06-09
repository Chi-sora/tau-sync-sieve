# Theory summary

## Core identity

The central identity is:

```text
x_s(N,j) = N - q_s(j)
```

For source stream `a` and target stream `b`:

```text
delta_x(j,t)
  = x_b(N_b,j) - x_a(N_a,t)
```

If:

```text
delta_x(j,t) = 0
```

then:

```text
x_b(N_b,j) = x_a(N_a,t)
```

This equality transfers the verified `x` fact from source to target.

## g-sync transfer

If source has a Goldbach endpoint:

```text
G_a(N_a,t):
  Prime(q_a(t))
  and Prime(x_a(N_a,t))
```

and target q is prime at the forced position:

```text
Prime(q_b(j_forced))
```

and:

```text
delta_x(j_forced,t) = 0
```

then:

```text
Prime(x_b(N_b,j_forced))
```

because:

```text
x_b(N_b,j_forced) = x_a(N_a,t)
```

Therefore:

```text
G_b(N_b,j_forced)
tau_gb(N_b) <= j_forced
```

## valid_t as prime-pair condition

For the pair 55 -> 15:

```text
q_55(t) = 6t - 1
q_15(j) = 6j + 1
```

If:

```text
q_15(j_forced) = q_55(t) + B
```

then `valid_t` is:

```text
Prime(q_55(t))
and
Prime(q_55(t) + B)
```

For B=74:

```text
{6t-1, 6t+73}
```

are the two prime-side values.

## mod 6 layer

The mod 6 layer is structural:

```text
q_s(j) = 6j +/- 1
```

It is used for:

```text
1. q generation
2. C3 scan exclusion
3. orig mod 6 -> B
4. B -> j_forced
5. valid_t residue alignment
6. delta_x integer alignment
```

It is not a tau.

### C3 scan exclusion

C3 inactivity is controlled by residue separation, not by the special condition
`N = 0 mod 3` alone.

```text
x_s(N,j) = N - q_s(j)

x_s(N,j) = 0 mod 3
  iff N = q_s(j) mod 3
```

Thus:

```text
C3 is inactive inside a scan
  iff N != q_s(j) mod 3
```

The checked scan configuration satisfies this condition by construction.
The commonly used case `N = 0 mod 3` with `q_s(j)=6j +/- 1` is a sufficient
example of the same rule.

C3 may still be meaningful in companion classification outside the scan setting.

## Lapse as arithmetic no-hit prefix

For `tau_gs(N) = t`, the prefix `j < t` satisfies:

```text
not G_s(N,j)
```

This is decomposed into:

```text
q_fail
x_killed_lapse
x_prough_lapse
```

The congruence behind the x-side sieve is:

```text
x_s(N,j) = N - q_s(j)

x_s(N,j) = 0 mod p
  iff
q_s(j) = N mod p
```

Thus, small-prime killed positions are arithmetic residue classes.

## Endpoint-late evaluation rule

For tau-vector scans and sync-audit scans:

```text
1. evaluate q_s(j)
2. compute x_s(N,j)
3. classify x_s(N,j)
4. record nearend tau values
5. record endpoint tau_g if q and x are prime
6. apply stopping rule
```

This avoids hiding nearend facts by stopping too early at an endpoint.

## Latent and observed sync

```text
latent C-sync:
  the arithmetic transfer places component C at j_forced

observed C-sync:
  the scan reaches j_forced and records C
```

For `g-sync`, the transferred component is the endpoint itself, so it is directly observed in the checked setting.

For nearend components, an earlier endpoint may stop the scan before `j_forced`. This causes observed failure without contradicting the latent arithmetic transfer.

## Dependency graph

A dependency graph is a directed graph:

```text
D = (V,E)
```

Nodes are predicates, tau values, lapse values, sync facts, tables, and caches.

An edge:

```text
A -> B
```

means:

```text
B requires A.
```

Purpose:

```text
split processing into passes without leaving required tau or component facts behind
```

Minimal order:

```text
1. precompute q and valid_t tables
2. endpoint pass
3. lapse pass
4. sync pass
5. optional nearend audit pass
```


## Relation to packed full-index lookup

The packed index is a finite lookup layer.

```text
x_s(N,j) = N - q_s(j)
```

This identity is unchanged.  The index may provide class facts for integer
values, but a tau-sync result still requires its predecessor facts to be known,
certified, or explicitly unresolved/unaudited.
