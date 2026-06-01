/*
 * tau-sync sieve
 * Author: Chisora
 * SPDX-License-Identifier: MIT
 */

/* tau_sync_sieve_full_validation_v7_oneline_progress.c
   ASCII-only Windows 11 + MinGW64 + cmd.exe validation kit.

   Purpose:
     Production-style validator for the tau-sync sieve prototype.

   Fixed requirements:
     JMAX = 5000 by default.
     classify5() is cbrt-based; no Pollard, no SQUFOF.
     scan_g() / scan_tv() do not check x % 3 inside scan.
     C3 is companion-only under N == 0 mod 3 and q=6j+/-1.
     endpoint-late rule is used inside scan_tv().
     g-sync is validated through V_B valid-t and delta_x=0.
     tau/lapse substitution is used in validation mode:
       - j < tau_g with q prime implies x composite by lapse.
       - target tau_g is implied by source G, delta_x=0, and target q prime.
       - direct target scanning is optional with TAU_DIRECT_TARGET_CHECK=1.
     per-base CSV append and flush.
     RAM caches are default. Binary cache files are saved/loaded in this kit's own format.

   Build:
     gcc -O3 -march=native -std=c11 -Wall -Wextra -DNDEBUG -fopenmp tau_sync_sieve_full_validation.c -o tau_sync_sieve_full_validation.exe

   Run:
     tau_sync_sieve_full_validation.exe selftest
     tau_sync_sieve_full_validation.exe base_external_validation_inputs.csv tau_sync_sieve_full_results.csv 16 pilot
     tau_sync_sieve_full_validation.exe base_external_validation_inputs.csv tau_sync_sieve_full_results.csv 16 full
*/

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef _OPENMP
#include <omp.h>
#else
static int omp_get_thread_num(void) { return 0; }
static int omp_get_max_threads(void) { return 1; }
#endif

#if defined(__GNUC__)
typedef __uint128_t u128;
#else
#error GCC/Clang with unsigned __int128 is required.
#endif

typedef uint64_t u64;

#ifndef JMAX_DEFAULT
#define JMAX_DEFAULT 5000
#endif
#ifndef P_DEF
#define P_DEF 193
#endif
#define PRIME_TABLE_LIMIT 2642250

#ifndef PRIME_CACHE_POW2
#define PRIME_CACHE_POW2 22
#endif
#ifndef CLASS_CACHE_POW2
#define CLASS_CACHE_POW2 22
#endif
#define PRIME_CACHE_SIZE (1U << PRIME_CACHE_POW2)
#define CLASS_CACHE_SIZE (1U << CLASS_CACHE_POW2)

#define MAX_BASES 4096
#define MAX_LINE 4096
#define MAX_T_INDEX (JMAX_DEFAULT + 1)
#define PAIR_N 4
#define B_N 4
#define D_N 10

enum XClass {
    X_INVALID = 0,
    X_P       = 1,
    X_S_PR    = 2,
    X_S_K     = 3,
    X_A_PR    = 4,
    X_A_K     = 5,
    X_C3      = 6
};

typedef struct { int cls; int factor; int cofactor_prime_needed; } XInfo;
typedef struct { int hit; int j; u64 q; u64 x; } GHit;
typedef struct {
    int tau_q_ne;
    int tau_x_ne_raw;
    int tau_g;
    int tau_semi_ne;
    int tau_shadow_ne;
    int tau_eclipse_ne;
    int tau_fury_ne;
    int tau_c3_ne;
} TauVector;

typedef struct { u64 key; unsigned char val; } PrimeCacheEntry;
typedef struct { u64 key; unsigned char cls; unsigned short factor; unsigned char need; } ClassCacheEntry;

static int *g_primes = NULL;
static int g_prime_count = 0;
static PrimeCacheEntry *g_prime_cache = NULL;
static ClassCacheEntry *g_class_cache = NULL;
static u64 g_prime_cache_hits = 0, g_prime_cache_misses = 0;
static u64 g_class_cache_hits = 0, g_class_cache_misses = 0;

static u64 hash_u64(u64 x) {
    x ^= x >> 33;
    x *= 0xff51afd7ed558ccdULL;
    x ^= x >> 33;
    x *= 0xc4ceb9fe1a85ec53ULL;
    x ^= x >> 33;
    return x;
}

static void init_caches(void) {
    if (!g_prime_cache) {
        g_prime_cache = (PrimeCacheEntry *)calloc((size_t)PRIME_CACHE_SIZE, sizeof(PrimeCacheEntry));
        if (!g_prime_cache) { fprintf(stderr, "prime cache allocation failed\n"); exit(2); }
    }
    if (!g_class_cache) {
        g_class_cache = (ClassCacheEntry *)calloc((size_t)CLASS_CACHE_SIZE, sizeof(ClassCacheEntry));
        if (!g_class_cache) { fprintf(stderr, "class cache allocation failed\n"); exit(2); }
    }
}

static u64 mod_mul(u64 a, u64 b, u64 m) { return (u64)(((u128)a * (u128)b) % (u128)m); }
static u64 mod_pow(u64 a, u64 d, u64 m) {
    u64 r = 1;
    while (d) {
        if (d & 1ULL) r = mod_mul(r, a, m);
        a = mod_mul(a, a, m);
        d >>= 1ULL;
    }
    return r;
}

static int is_prime64_raw(u64 n) {
    static const u64 small[] = {2ULL,3ULL,5ULL,7ULL,11ULL,13ULL,17ULL,19ULL,23ULL,29ULL,31ULL,37ULL};
    static const u64 bases[] = {2ULL,325ULL,9375ULL,28178ULL,450775ULL,9780504ULL,1795265022ULL};
    if (n < 2ULL) return 0;
    for (unsigned i = 0; i < sizeof(small)/sizeof(small[0]); i++) {
        if (n == small[i]) return 1;
        if (n % small[i] == 0ULL) return 0;
    }
    u64 d = n - 1ULL;
    int s = 0;
    while ((d & 1ULL) == 0ULL) { d >>= 1ULL; s++; }
    for (unsigned i = 0; i < sizeof(bases)/sizeof(bases[0]); i++) {
        u64 a = bases[i] % n;
        if (a == 0ULL) continue;
        u64 x = mod_pow(a, d, n);
        if (x == 1ULL || x == n - 1ULL) continue;
        int ok = 0;
        for (int r = 1; r < s; r++) {
            x = mod_mul(x, x, n);
            if (x == n - 1ULL) { ok = 1; break; }
        }
        if (!ok) return 0;
    }
    return 1;
}

static int is_prime64_cached_nolock(u64 n) {
    init_caches();
    u64 h = hash_u64(n);
    unsigned idx = (unsigned)(h & (PRIME_CACHE_SIZE - 1U));
    for (unsigned probe = 0; probe < 8U; probe++) {
        PrimeCacheEntry *e = &g_prime_cache[(idx + probe) & (PRIME_CACHE_SIZE - 1U)];
        if (e->val == 0U) {
            int r = is_prime64_raw(n);
            e->key = n;
            e->val = (unsigned char)(r ? 2U : 1U);
            g_prime_cache_misses++;
            return r;
        }
        if (e->key == n) {
            g_prime_cache_hits++;
            return e->val == 2U;
        }
    }
    g_prime_cache_misses++;
    return is_prime64_raw(n);
}

static int is_prime64_cached(u64 n) {
#ifdef _OPENMP
    int r;
#pragma omp critical(tau_prime_cache)
    { r = is_prime64_cached_nolock(n); }
    return r;
#else
    return is_prime64_cached_nolock(n);
#endif
}

static u64 icbrt_u64(u64 n) {
    u64 lo = 0, hi = 3000000ULL;
    while (lo + 1ULL < hi) {
        u64 mid = lo + (hi - lo) / 2ULL;
        u128 cube = (u128)mid * (u128)mid * (u128)mid;
        if (cube <= (u128)n) lo = mid;
        else hi = mid;
    }
    return lo;
}

static void build_prime_table(void) {
    int limit = PRIME_TABLE_LIMIT;
    unsigned char *mark = (unsigned char *)calloc((size_t)limit + 1U, 1U);
    if (!mark) { fprintf(stderr, "prime table allocation failed\n"); exit(2); }
    for (int i = 2; (int64_t)i * (int64_t)i <= limit; i++) {
        if (!mark[i]) for (int j = i * i; j <= limit; j += i) mark[j] = 1U;
    }
    int cnt = 0;
    for (int i = 2; i <= limit; i++) if (!mark[i]) cnt++;
    g_primes = (int *)malloc((size_t)cnt * sizeof(int));
    if (!g_primes) { fprintf(stderr, "prime table allocation failed\n"); exit(2); }
    int k = 0;
    for (int i = 2; i <= limit; i++) if (!mark[i]) g_primes[k++] = i;
    g_prime_count = cnt;
    free(mark);
}

static int has_factor_up_to_cbrt(u64 n, int start_after_pdef, int *factor_out) {
    u64 r3 = icbrt_u64(n);
    for (int i = 0; i < g_prime_count; i++) {
        int p = g_primes[i];
        if ((u64)p > r3) break;
        if (start_after_pdef && p <= P_DEF) continue;
        if (n % (u64)p == 0ULL) { if (factor_out) *factor_out = p; return 1; }
    }
    if (factor_out) *factor_out = 0;
    return 0;
}

static int killed5_factor(u64 x) {
    for (int i = 0; i < g_prime_count; i++) {
        int p = g_primes[i];
        if (p < 5) continue;
        if (p > P_DEF) break;
        if (x > (u64)p && x % (u64)p == 0ULL) return p;
    }
    return 0;
}

static int cofactor_is_prime_final(u64 y, int *residual_isprime_needed) {
    if (residual_isprime_needed) *residual_isprime_needed = 0;
    if (y < 2ULL) return 0;
    if (y == 2ULL || y == 3ULL) return 1;
    if ((y & 1ULL) == 0ULL) return 0;
    if (y % 3ULL == 0ULL) return 0;
    int f = 0;
    if (has_factor_up_to_cbrt(y, 0, &f)) return 0;
    if (residual_isprime_needed) *residual_isprime_needed = 1;
    return is_prime64_cached(y);
}

static XInfo classify5_raw(u64 x, int companion_mode) {
    XInfo info; info.cls = X_INVALID; info.factor = 0; info.cofactor_prime_needed = 0;
    if (companion_mode && x > 3ULL && x % 3ULL == 0ULL) { info.cls = X_C3; info.factor = 3; return info; }
    int kp = killed5_factor(x);
    if (kp) {
        u64 y = x / (u64)kp;
        int need = 0;
        int yp = cofactor_is_prime_final(y, &need);
        info.factor = kp; info.cofactor_prime_needed = need; info.cls = yp ? X_S_K : X_A_K;
        return info;
    }
    if (is_prime64_cached(x)) { info.cls = X_P; return info; }
    int f = 0;
    if (!has_factor_up_to_cbrt(x, 1, &f)) { info.cls = X_S_PR; return info; }
    u64 y = x / (u64)f;
    info.factor = f;
    info.cls = is_prime64_cached(y) ? X_S_PR : X_A_PR;
    return info;
}


static XInfo classify5_known_composite_raw(u64 x) {
    XInfo info; info.cls = X_INVALID; info.factor = 0; info.cofactor_prime_needed = 0;
    int kp = killed5_factor(x);
    if (kp) {
        u64 y = x / (u64)kp;
        int need = 0;
        int yp = cofactor_is_prime_final(y, &need);
        info.factor = kp;
        info.cofactor_prime_needed = need;
        info.cls = yp ? X_S_K : X_A_K;
        return info;
    }
    /* Known-composite P-rough path.
       This function is used only when lapse proves x is composite:
         j < tau_g and q_s(j) is prime.
       Therefore it deliberately skips is_prime(x). */
    int f = 0;
    if (!has_factor_up_to_cbrt(x, 1, &f)) { info.cls = X_S_PR; return info; }
    u64 y = x / (u64)f;
    info.factor = f;
    info.cls = is_prime64_cached(y) ? X_S_PR : X_A_PR;
    return info;
}

static XInfo classify5_known_composite_cached_nolock(u64 x) {
    init_caches();
    u64 h = hash_u64(x ^ 0x517cc1b727220a95ULL);
    unsigned idx = (unsigned)(h & (CLASS_CACHE_SIZE - 1U));
    for (unsigned probe = 0; probe < 8U; probe++) {
        ClassCacheEntry *e = &g_class_cache[(idx + probe) & (CLASS_CACHE_SIZE - 1U)];
        if (e->cls == 0U) {
            XInfo ci = classify5_known_composite_raw(x);
            e->key = x;
            e->cls = (unsigned char)ci.cls;
            e->factor = (unsigned short)((ci.factor > 65535) ? 0 : ci.factor);
            e->need = (unsigned char)ci.cofactor_prime_needed;
            g_class_cache_misses++;
            return ci;
        }
        if (e->key == x) {
            XInfo ci; ci.cls = (int)e->cls; ci.factor = (int)e->factor; ci.cofactor_prime_needed = (int)e->need;
            g_class_cache_hits++;
            return ci;
        }
    }
    g_class_cache_misses++;
    return classify5_known_composite_raw(x);
}

static XInfo classify5_known_composite_cached(u64 x) {
#ifdef _OPENMP
    XInfo ci;
#pragma omp critical(tau_class_cache)
    { ci = classify5_known_composite_cached_nolock(x); }
    return ci;
#else
    return classify5_known_composite_cached_nolock(x);
#endif
}

static XInfo classify5_scan_cached_nolock(u64 x) {
    init_caches();
    u64 h = hash_u64(x ^ 0x9e3779b97f4a7c15ULL);
    unsigned idx = (unsigned)(h & (CLASS_CACHE_SIZE - 1U));
    for (unsigned probe = 0; probe < 8U; probe++) {
        ClassCacheEntry *e = &g_class_cache[(idx + probe) & (CLASS_CACHE_SIZE - 1U)];
        if (e->cls == 0U) {
            XInfo ci = classify5_raw(x, 0);
            e->key = x;
            e->cls = (unsigned char)ci.cls;
            e->factor = (unsigned short)((ci.factor > 65535) ? 0 : ci.factor);
            e->need = (unsigned char)ci.cofactor_prime_needed;
            g_class_cache_misses++;
            return ci;
        }
        if (e->key == x) {
            XInfo ci; ci.cls = (int)e->cls; ci.factor = (int)e->factor; ci.cofactor_prime_needed = (int)e->need;
            g_class_cache_hits++;
            return ci;
        }
    }
    g_class_cache_misses++;
    return classify5_raw(x, 0);
}

static XInfo classify5_scan_cached(u64 x) {
#ifdef _OPENMP
    XInfo ci;
#pragma omp critical(tau_class_cache)
    { ci = classify5_scan_cached_nolock(x); }
    return ci;
#else
    return classify5_scan_cached_nolock(x);
#endif
}

static u64 q_at(int j, int sign) { return (sign >= 0) ? (6ULL * (u64)j + 1ULL) : (6ULL * (u64)j - 1ULL); }

static int stream_sign(int s) { return (s == 11 || s == 15) ? +1 : -1; }

static const int PAIR_SRC[PAIR_N] = {55,51,11,15};
static const int PAIR_TGT[PAIR_N] = {15,11,51,55};
static const char *PAIR_NAME[PAIR_N] = {"55_to_15","51_to_11","11_to_51","15_to_55"};

static int B_index(int B) {
    if (B == 68) return 0;
    if (B == 70) return 1;
    if (B == 74) return 2;
    if (B == 76) return 3;
    return -1;
}
static int B_value_from_index(int i) { return (i==0)?68:(i==1)?70:(i==2)?74:76; }

static int B_for_pair(int source, int target, int orig_mod6) {
    if (source == 55 && target == 15) return (orig_mod6 == 0 || orig_mod6 == 5) ? 68 : 74;
    if (source == 51 && target == 11) return (orig_mod6 == 1 || orig_mod6 == 2) ? 68 : 74;
    if (source == 15 && target == 55) return (orig_mod6 == 0 || orig_mod6 == 5) ? 76 : 70;
    if (source == 11 && target == 51) return (orig_mod6 == 1 || orig_mod6 == 2) ? 76 : 70;
    return 0;
}

static int j_from_q_target(u64 q, int target_sign) {
    if (target_sign >= 0) {
        if (q < 7ULL) return 0;
        if ((q - 1ULL) % 6ULL != 0ULL) return 0;
        return (int)((q - 1ULL) / 6ULL);
    } else {
        if (q < 5ULL) return 0;
        if ((q + 1ULL) % 6ULL != 0ULL) return 0;
        return (int)((q + 1ULL) / 6ULL);
    }
}

static u64 align_residue(u64 base, int residue) {
    u64 r = base % 6ULL;
    return base + ((u64)residue + 6ULL - r) % 6ULL;
}

static u64 stream_start(u64 origin, int stream) {
    if (stream == 11 || stream == 15) return align_residue(origin, 0);
    return align_residue(align_residue(origin, 0), 4);
}

static GHit scan_g(u64 N, int sign, int Jmax) {
    GHit out; out.hit = 0; out.j = 0; out.q = 0; out.x = 0;
    for (int j = 1; j <= Jmax; j++) {
        u64 q = q_at(j, sign);
        if (q >= N) break;
        if (!is_prime64_cached(q)) continue;
        u64 x = N - q;
        if (is_prime64_cached(x)) { out.hit = 1; out.j = j; out.q = q; out.x = x; return out; }
    }
    return out;
}

static int tv_complete(const TauVector *tv) {
    return tv->tau_q_ne && tv->tau_x_ne_raw && tv->tau_g && tv->tau_semi_ne && tv->tau_shadow_ne && tv->tau_eclipse_ne && tv->tau_fury_ne;
}
static int tv_q_conditioned_complete(const TauVector *tv) {
    return tv->tau_q_ne && tv->tau_g && tv->tau_semi_ne && tv->tau_shadow_ne && tv->tau_eclipse_ne && tv->tau_fury_ne;
}
static TauVector scan_tv(u64 N, int sign, int Jmax) {
    TauVector tv; memset(&tv, 0, sizeof(tv));
    for (int j = 1; j <= Jmax; j++) {
        if (tv_complete(&tv)) break;
        u64 q = q_at(j, sign);
        if (q >= N) break;
        u64 x = N - q;
        int q_needed = !tv_q_conditioned_complete(&tv);
        int qprime = 0;
        if (q_needed) {
            qprime = is_prime64_cached(q);
            if (qprime && tv.tau_q_ne == 0) tv.tau_q_ne = j;
        }
        if (!q_needed) {
            if (tv.tau_x_ne_raw == 0 && is_prime64_cached(x)) tv.tau_x_ne_raw = j;
            continue;
        }
        if (!qprime) {
            if (tv.tau_x_ne_raw == 0 && is_prime64_cached(x)) tv.tau_x_ne_raw = j;
            continue;
        }
        XInfo ci = classify5_scan_cached(x);
        if (ci.cls == X_P && tv.tau_x_ne_raw == 0) tv.tau_x_ne_raw = j;
        if (ci.cls == X_P && tv.tau_g == 0) tv.tau_g = j;
        if ((ci.cls == X_S_PR || ci.cls == X_S_K) && tv.tau_semi_ne == 0) tv.tau_semi_ne = j;
        if ((ci.cls == X_S_K || ci.cls == X_A_K) && tv.tau_shadow_ne == 0) tv.tau_shadow_ne = j;
        if (ci.cls == X_A_K && tv.tau_eclipse_ne == 0) tv.tau_eclipse_ne = j;
        if ((ci.cls == X_A_PR || ci.cls == X_A_K) && tv.tau_fury_ne == 0) tv.tau_fury_ne = j;
    }
    return tv;
}

typedef struct {
    char base_id[64];
    u64 N_base;
    int M_pilot;
    int M_full;
    int P;
    int T_shadow;
    int T_ecl;
    int Jmax_pilot;
    int Jmax_full;
} BaseRow;

typedef struct {
    u64 source_cases;
    u64 source_endpoint_missing;
    u64 sync_slots;
    u64 sync_success;
    u64 sync_fail;
    u64 invalid_j;
    u64 delta_fail;
    u64 qtarget_not_prime;
    u64 by_pair_slots[PAIR_N];
    u64 by_pair_fail[PAIR_N];
    u64 by_bt_slots[PAIR_N][B_N][MAX_T_INDEX];
    u64 by_bt_fail[PAIR_N][B_N][MAX_T_INDEX];
    u64 by_d_slots[PAIR_N][D_N];
    u64 by_d_fail[PAIR_N][D_N];
    u64 lapse_q_fail[PAIR_N];
    u64 lapse_q_prime_prefix[PAIR_N];
    u64 lapse_x_killed[PAIR_N];
    u64 lapse_x_prough[PAIR_N];
    u64 lapse_s_pr[PAIR_N];
    u64 lapse_a_pr[PAIR_N];
    u64 lapse_c3_scan[PAIR_N];
    u64 lapse_known_composite_uses[PAIR_N];
    u64 target_scan_skipped_by_sync;
    u64 target_direct_checks;
    u64 target_direct_fail;
} RunStats;

static void stats_add(RunStats *a, const RunStats *b) {
    a->source_cases += b->source_cases;
    a->source_endpoint_missing += b->source_endpoint_missing;
    a->sync_slots += b->sync_slots;
    a->sync_success += b->sync_success;
    a->sync_fail += b->sync_fail;
    a->invalid_j += b->invalid_j;
    a->delta_fail += b->delta_fail;
    a->qtarget_not_prime += b->qtarget_not_prime;
    a->target_scan_skipped_by_sync += b->target_scan_skipped_by_sync;
    a->target_direct_checks += b->target_direct_checks;
    a->target_direct_fail += b->target_direct_fail;
    for (int p=0;p<PAIR_N;p++) {
        a->by_pair_slots[p] += b->by_pair_slots[p];
        a->by_pair_fail[p] += b->by_pair_fail[p];
        a->lapse_q_fail[p] += b->lapse_q_fail[p];
        a->lapse_q_prime_prefix[p] += b->lapse_q_prime_prefix[p];
        a->lapse_x_killed[p] += b->lapse_x_killed[p];
        a->lapse_x_prough[p] += b->lapse_x_prough[p];
        a->lapse_s_pr[p] += b->lapse_s_pr[p];
        a->lapse_a_pr[p] += b->lapse_a_pr[p];
        a->lapse_c3_scan[p] += b->lapse_c3_scan[p];
        a->lapse_known_composite_uses[p] += b->lapse_known_composite_uses[p];
        for (int d=0;d<D_N;d++) { a->by_d_slots[p][d] += b->by_d_slots[p][d]; a->by_d_fail[p][d] += b->by_d_fail[p][d]; }
        for (int bi=0;bi<B_N;bi++) for (int t=0;t<MAX_T_INDEX;t++) { a->by_bt_slots[p][bi][t] += b->by_bt_slots[p][bi][t]; a->by_bt_fail[p][bi][t] += b->by_bt_fail[p][bi][t]; }
    }
}

static void lapse_prefix_audit(RunStats *s, int pair_idx, u64 N, int sign, int upto_j_exclusive) {
    for (int j = 1; j < upto_j_exclusive; j++) {
        u64 q = q_at(j, sign);
        if (q >= N) break;
        if (!is_prime64_cached(q)) { s->lapse_q_fail[pair_idx]++; continue; }
        s->lapse_q_prime_prefix[pair_idx]++;
        u64 x = N - q;
        if (x > 3ULL && x % 3ULL == 0ULL) s->lapse_c3_scan[pair_idx]++;
        int kf = killed5_factor(x);
        if (kf) { s->lapse_x_killed[pair_idx]++; continue; }
        s->lapse_x_prough[pair_idx]++;
        s->lapse_known_composite_uses[pair_idx]++;
        XInfo ci = classify5_known_composite_cached(x);
        if (ci.cls == X_S_PR) s->lapse_s_pr[pair_idx]++;
        else if (ci.cls == X_A_PR) s->lapse_a_pr[pair_idx]++;
    }
}

static void process_case(RunStats *s, u64 origin, int k, int d, int pair_idx, int Jmax) {
    int source = PAIR_SRC[pair_idx];
    int target = PAIR_TGT[pair_idx];
    int ss = stream_sign(source);
    int ts = stream_sign(target);
    int orig_mod6 = (int)(origin % 6ULL);
    int B = B_for_pair(source, target, orig_mod6);
    int bi = B_index(B);
    if (B == 0 || bi < 0) return;

    u64 src0 = stream_start(origin, source);
    u64 Nsource = src0 + 6ULL * (u64)(k + 12 + d);
    GHit gh = scan_g(Nsource, ss, Jmax);
    s->source_cases++;
    if (!gh.hit) { s->source_endpoint_missing++; return; }

    lapse_prefix_audit(s, pair_idx, Nsource, ss, gh.j);

    u64 qsrc = gh.q;
    u64 qtar = qsrc + (u64)B;
    int jforced = j_from_q_target(qtar, ts);
    if (jforced <= 0 || jforced > Jmax) { s->invalid_j++; return; }

    if (!is_prime64_cached(qtar)) { s->qtarget_not_prime++; return; }

    u64 Ntarget = Nsource + (u64)B;
    u64 xsource = gh.x;
    u64 xtarget = Ntarget - qtar;
    if (xsource != xtarget) { s->delta_fail++; return; }

    int ok = 1;
    const char *direct_env = getenv("TAU_DIRECT_TARGET_CHECK");
    if (direct_env && strcmp(direct_env, "1") == 0) {
        GHit tgt = scan_g(Ntarget, ts, jforced);
        s->target_direct_checks++;
        ok = (tgt.hit && tgt.j <= jforced);
        if (!ok) s->target_direct_fail++;
    } else {
        /* tau/lapse substitution: source G + delta_x=0 + target q prime implies target G at jforced.
           Therefore tau_g_target <= jforced, and a full target scan is unnecessary. */
        s->target_scan_skipped_by_sync++;
    }

    s->sync_slots++;
    s->by_pair_slots[pair_idx]++;
    s->by_d_slots[pair_idx][d]++;
    if (gh.j >= 0 && gh.j < MAX_T_INDEX) s->by_bt_slots[pair_idx][bi][gh.j]++;
    if (ok) {
        s->sync_success++;
    } else {
        s->sync_fail++;
        s->by_pair_fail[pair_idx]++;
        s->by_d_fail[pair_idx][d]++;
        if (gh.j >= 0 && gh.j < MAX_T_INDEX) s->by_bt_fail[pair_idx][bi][gh.j]++;
    }
}

static int parse_base_csv(const char *path, BaseRow *rows, int maxrows) {
    FILE *f = fopen(path, "r");
    if (!f) { fprintf(stderr, "cannot open input: %s\n", path); return -1; }
    char line[MAX_LINE];
    if (!fgets(line, sizeof(line), f)) { fclose(f); return 0; }
    int n = 0;
    while (fgets(line, sizeof(line), f) && n < maxrows) {
        char *tok[16]; int nt = 0;
        char *p = strtok(line, ",\r\n");
        while (p && nt < 16) { tok[nt++] = p; p = strtok(NULL, ",\r\n"); }
        if (nt < 9) continue;
        strncpy(rows[n].base_id, tok[0], sizeof(rows[n].base_id)-1);
        rows[n].base_id[sizeof(rows[n].base_id)-1] = 0;
        rows[n].N_base = (u64)strtoull(tok[1], NULL, 10);
        rows[n].M_pilot = atoi(tok[2]);
        rows[n].M_full = atoi(tok[3]);
        rows[n].P = atoi(tok[4]);
        rows[n].T_shadow = atoi(tok[5]);
        rows[n].T_ecl = atoi(tok[6]);
        rows[n].Jmax_pilot = atoi(tok[7]);
        rows[n].Jmax_full = atoi(tok[8]);
        n++;
    }
    fclose(f);
    return n;
}

static void write_header(FILE *f) {
    fprintf(f, "section,base_id,source,target,B,t,d,metric,value,extra\n");
}
static void write_stats(FILE *f, const char *base_id, const RunStats *s) {
    fprintf(f, "summary,%s,0,0,0,0,0,source_cases,%llu,\n", base_id, (unsigned long long)s->source_cases);
    fprintf(f, "summary,%s,0,0,0,0,0,source_endpoint_missing,%llu,\n", base_id, (unsigned long long)s->source_endpoint_missing);
    fprintf(f, "summary,%s,0,0,0,0,0,sync_slots,%llu,\n", base_id, (unsigned long long)s->sync_slots);
    fprintf(f, "summary,%s,0,0,0,0,0,sync_success,%llu,\n", base_id, (unsigned long long)s->sync_success);
    fprintf(f, "summary,%s,0,0,0,0,0,sync_fail,%llu,\n", base_id, (unsigned long long)s->sync_fail);
    fprintf(f, "summary,%s,0,0,0,0,0,invalid_j,%llu,\n", base_id, (unsigned long long)s->invalid_j);
    fprintf(f, "summary,%s,0,0,0,0,0,delta_fail,%llu,\n", base_id, (unsigned long long)s->delta_fail);
    fprintf(f, "summary,%s,0,0,0,0,0,qtarget_not_prime,%llu,\n", base_id, (unsigned long long)s->qtarget_not_prime);
    fprintf(f, "summary,%s,0,0,0,0,0,target_scan_skipped_by_sync,%llu,\n", base_id, (unsigned long long)s->target_scan_skipped_by_sync);
    fprintf(f, "summary,%s,0,0,0,0,0,target_direct_checks,%llu,\n", base_id, (unsigned long long)s->target_direct_checks);
    fprintf(f, "summary,%s,0,0,0,0,0,target_direct_fail,%llu,\n", base_id, (unsigned long long)s->target_direct_fail);
    for (int p=0;p<PAIR_N;p++) {
        int src=PAIR_SRC[p], tgt=PAIR_TGT[p];
        fprintf(f, "by_pair,%s,%d,%d,0,0,0,slots,%llu,%s\n", base_id, src,tgt,(unsigned long long)s->by_pair_slots[p],PAIR_NAME[p]);
        fprintf(f, "by_pair,%s,%d,%d,0,0,0,fail,%llu,%s\n", base_id, src,tgt,(unsigned long long)s->by_pair_fail[p],PAIR_NAME[p]);
        fprintf(f, "lapse,%s,%d,%d,0,0,0,q_fail,%llu,%s\n", base_id,src,tgt,(unsigned long long)s->lapse_q_fail[p],PAIR_NAME[p]);
        fprintf(f, "lapse,%s,%d,%d,0,0,0,q_prime_prefix,%llu,%s\n", base_id,src,tgt,(unsigned long long)s->lapse_q_prime_prefix[p],PAIR_NAME[p]);
        fprintf(f, "lapse,%s,%d,%d,0,0,0,x_killed,%llu,%s\n", base_id,src,tgt,(unsigned long long)s->lapse_x_killed[p],PAIR_NAME[p]);
        fprintf(f, "lapse,%s,%d,%d,0,0,0,x_prough,%llu,%s\n", base_id,src,tgt,(unsigned long long)s->lapse_x_prough[p],PAIR_NAME[p]);
        fprintf(f, "lapse,%s,%d,%d,0,0,0,S_PR_lapse,%llu,%s\n", base_id,src,tgt,(unsigned long long)s->lapse_s_pr[p],PAIR_NAME[p]);
        fprintf(f, "lapse,%s,%d,%d,0,0,0,A_PR_lapse,%llu,%s\n", base_id,src,tgt,(unsigned long long)s->lapse_a_pr[p],PAIR_NAME[p]);
        fprintf(f, "lapse,%s,%d,%d,0,0,0,C3_scan,%llu,%s\n", base_id,src,tgt,(unsigned long long)s->lapse_c3_scan[p],PAIR_NAME[p]);
        fprintf(f, "lapse,%s,%d,%d,0,0,0,known_composite_uses,%llu,%s\n", base_id,src,tgt,(unsigned long long)s->lapse_known_composite_uses[p],PAIR_NAME[p]);
        for (int d=0; d<D_N; d++) {
            if (s->by_d_slots[p][d] || s->by_d_fail[p][d]) {
                fprintf(f, "by_d,%s,%d,%d,0,0,%d,slots,%llu,%s\n", base_id,src,tgt,d,(unsigned long long)s->by_d_slots[p][d],PAIR_NAME[p]);
                fprintf(f, "by_d,%s,%d,%d,0,0,%d,fail,%llu,%s\n", base_id,src,tgt,d,(unsigned long long)s->by_d_fail[p][d],PAIR_NAME[p]);
            }
        }
        for (int bi=0; bi<B_N; bi++) {
            int B = B_value_from_index(bi);
            for (int t=1; t<MAX_T_INDEX; t++) {
                if (s->by_bt_slots[p][bi][t] || s->by_bt_fail[p][bi][t]) {
                    fprintf(f, "by_B_t,%s,%d,%d,%d,%d,0,slots,%llu,%s\n", base_id,src,tgt,B,t,(unsigned long long)s->by_bt_slots[p][bi][t],PAIR_NAME[p]);
                    fprintf(f, "by_B_t,%s,%d,%d,%d,%d,0,fail,%llu,%s\n", base_id,src,tgt,B,t,(unsigned long long)s->by_bt_fail[p][bi][t],PAIR_NAME[p]);
                }
            }
        }
    }
    fprintf(f, "cache,%s,0,0,0,0,0,prime_cache_hits,%llu,\n", base_id, (unsigned long long)g_prime_cache_hits);
    fprintf(f, "cache,%s,0,0,0,0,0,prime_cache_misses,%llu,\n", base_id, (unsigned long long)g_prime_cache_misses);
    fprintf(f, "cache,%s,0,0,0,0,0,class_cache_hits,%llu,\n", base_id, (unsigned long long)g_class_cache_hits);
    fprintf(f, "cache,%s,0,0,0,0,0,class_cache_misses,%llu,\n", base_id, (unsigned long long)g_class_cache_misses);
}

static void save_cache_files(void) {
    init_caches();
    FILE *f = fopen("tau_sync_prime_cache.bin", "wb");
    if (f) {
        const char magic[8] = {'T','S','P','R','I','M','E','2'};
        fwrite(magic,1,8,f);
        unsigned x = PRIME_CACHE_SIZE;
        fwrite(&x,sizeof(x),1,f);
        fwrite(g_prime_cache,sizeof(PrimeCacheEntry),(size_t)PRIME_CACHE_SIZE,f);
        fclose(f);
    }
    f = fopen("tau_sync_class_cache.bin", "wb");
    if (f) {
        const char magic[8] = {'T','S','C','L','A','S','S','2'};
        fwrite(magic,1,8,f);
        unsigned x = CLASS_CACHE_SIZE;
        fwrite(&x,sizeof(x),1,f);
        fwrite(g_class_cache,sizeof(ClassCacheEntry),(size_t)CLASS_CACHE_SIZE,f);
        fclose(f);
    }
}

static void load_cache_files(void) {
    init_caches();
    FILE *f = fopen("tau_sync_prime_cache.bin", "rb");
    if (f) {
        char magic[8]; unsigned x=0;
        if (fread(magic,1,8,f)==8 && memcmp(magic,"TSPRIME2",8)==0 && fread(&x,sizeof(x),1,f)==1 && x==PRIME_CACHE_SIZE) {
            fread(g_prime_cache,sizeof(PrimeCacheEntry),(size_t)PRIME_CACHE_SIZE,f);
            fprintf(stderr,"loaded tau_sync_prime_cache.bin\n");
        }
        fclose(f);
    }
    f = fopen("tau_sync_class_cache.bin", "rb");
    if (f) {
        char magic[8]; unsigned x=0;
        if (fread(magic,1,8,f)==8 && memcmp(magic,"TSCLASS2",8)==0 && fread(&x,sizeof(x),1,f)==1 && x==CLASS_CACHE_SIZE) {
            fread(g_class_cache,sizeof(ClassCacheEntry),(size_t)CLASS_CACHE_SIZE,f);
            fprintf(stderr,"loaded tau_sync_class_cache.bin\n");
        }
        fclose(f);
    }
}

static int run_base(const BaseRow *r, int use_full, int threads, FILE *outf, int ibase, int nbase, RunStats *global) {
    int M = use_full ? r->M_full : r->M_pilot;
    int Jmax = use_full ? r->Jmax_full : r->Jmax_pilot;
    if (Jmax <= 0 || Jmax > JMAX_DEFAULT) Jmax = JMAX_DEFAULT;
    RunStats *locals = (RunStats *)calloc((size_t)threads, sizeof(RunStats));
    if (!locals) { fprintf(stderr,"stats allocation failed\n"); return 1; }
    fprintf(stderr,"base %d/%d %s M=%d Jmax=%d threads=%d\n", ibase+1,nbase,r->base_id,M,Jmax,threads);
    fflush(stderr);
    clock_t t0 = clock();

    int completed_k = 0;
    int next_progress_pct = 0;

#ifdef _OPENMP
#pragma omp parallel for schedule(dynamic, 16) num_threads(threads)
#endif
    for (int k = 0; k < M; k++) {
        int tid = omp_get_thread_num();
        if (tid < 0 || tid >= threads) tid = 0;
        for (int d=0; d<D_N; d++) {
            for (int p=0; p<PAIR_N; p++) process_case(&locals[tid], r->N_base, k, d, p, Jmax);
        }

        int done_now;
#ifdef _OPENMP
#pragma omp atomic capture
#endif
        done_now = ++completed_k;

        int pct_i = (M > 0) ? (int)((100LL * (long long)done_now) / (long long)M) : 100;
        if (pct_i >= next_progress_pct || done_now == M) {
#ifdef _OPENMP
#pragma omp critical(tau_progress)
#endif
            {
                if (pct_i >= next_progress_pct || done_now == M) {
                    double pct = (M > 0) ? (100.0 * (double)done_now / (double)M) : 100.0;
                    double sec_now = (double)(clock() - t0) / (double)CLOCKS_PER_SEC;
                    fprintf(stderr,"\rprogress base=%s pct=%.2f%% k=%d/%d elapsed_cpu=%.2f     ", r->base_id, pct, done_now, M, sec_now);
                    fflush(stderr);
                    next_progress_pct = pct_i + 1;
                }
            }
        }
    }

    fprintf(stderr,"\n");
    fflush(stderr);

    RunStats s; memset(&s,0,sizeof(s));
    for (int i=0;i<threads;i++) stats_add(&s, &locals[i]);
    free(locals);
    write_stats(outf, r->base_id, &s);
    fflush(outf);
    stats_add(global, &s);
    save_cache_files();
    double sec = (double)(clock() - t0) / (double)CLOCKS_PER_SEC;
    fprintf(stderr,"finished base=%s cpu_seconds=%.2f sync_slots=%llu fail=%llu\n", r->base_id, sec, (unsigned long long)s.sync_slots, (unsigned long long)s.sync_fail);
    return 0;
}

static int selftest(void) {
    int errors = 0;
    XInfo ci;
    ci = classify5_raw(101ULL,0); if (ci.cls != X_P) errors++;
    ci = classify5_raw(3ULL*101ULL,1); if (ci.cls != X_C3) errors++;
    ci = classify5_raw(5ULL*101ULL,0); if (ci.cls != X_S_K) errors++;
    ci = classify5_raw(5ULL*101ULL*103ULL,0); if (ci.cls != X_A_K) errors++;
    ci = classify5_raw(1000003ULL*1000033ULL,0); if (ci.cls != X_S_PR) errors++;
    ci = classify5_known_composite_raw(1000003ULL*1000033ULL); if (ci.cls != X_S_PR) errors++;
    ci = classify5_known_composite_raw(1000003ULL*1000033ULL*1000037ULL); if (ci.cls != X_A_PR) errors++;
    u64 N = 1000000000000000000ULL;
    TauVector tv = scan_tv(N, +1, JMAX_DEFAULT);
    if (tv.tau_c3_ne != 0) errors++;
    int B = B_for_pair(55,15,4);
    if (B != 74) errors++;
    if (errors) { printf("selftest,ok,0\n"); return 1; }
    printf("selftest,ok,1\n");
    printf("JMAX,%d\n", JMAX_DEFAULT);
    printf("P_DEF,%d\n", P_DEF);
    printf("prime_table_count,%d\n", g_prime_count);
    printf("openmp_threads_max,%d\n", omp_get_max_threads());
    return 0;
}

int main(int argc, char **argv) {
    setvbuf(stderr, NULL, _IONBF, 0);
    setvbuf(stdout, NULL, _IONBF, 0);
    build_prime_table();
    init_caches();
    if (argc >= 2 && strcmp(argv[1], "selftest") == 0) return selftest();
    if (argc < 5) {
        fprintf(stderr,"usage:\n");
        fprintf(stderr,"  %s selftest\n", argv[0]);
        fprintf(stderr,"  %s <input.csv> <output.csv> <threads> <pilot|full>\n", argv[0]);
        return 1;
    }
    const char *inpath = argv[1];
    const char *outpath = argv[2];
    int threads = atoi(argv[3]);
    if (threads <= 0) threads = 1;
    if (threads > 64) threads = 64;
    int use_full = (strcmp(argv[4], "full") == 0);

    /* v3: create the CSV immediately.
       Run and check are separate, and even an early failure leaves a diagnostic CSV. */
    FILE *outf = fopen(outpath, "w");
    if (!outf) { fprintf(stderr,"cannot open output: %s\n", outpath); return 2; }
    write_header(outf);
    fprintf(outf,"run_status,global,0,0,0,0,0,started,1,mode=%s threads=%d input=%s version=v7_oneline_progress\n", use_full ? "full" : "pilot", threads, inpath);
    fflush(outf);

    BaseRow *rows = (BaseRow *)calloc(MAX_BASES, sizeof(BaseRow));
    if (!rows) {
        fprintf(stderr,"base row allocation failed\n");
        fprintf(outf,"run_status,global,0,0,0,0,0,error,1,base_row_allocation_failed\n");
        fclose(outf);
        return 2;
    }
    int nbase = parse_base_csv(inpath, rows, MAX_BASES);
    if (nbase <= 0) {
        fprintf(stderr,"no bases found or input not readable: %s\n", inpath);
        fprintf(outf,"run_status,global,0,0,0,0,0,error,1,no_bases_or_input_not_readable\n");
        fclose(outf);
        free(rows);
        return 2;
    }
    load_cache_files();
    RunStats global; memset(&global,0,sizeof(global));
    for (int i=0;i<nbase;i++) {
        if (run_base(&rows[i], use_full, threads, outf, i, nbase, &global) != 0) {
            fprintf(outf,"run_status,global,0,0,0,0,0,error,1,run_base_failed\n");
            fflush(outf);
            fclose(outf);
            free(rows);
            return 3;
        }
    }
    write_stats(outf, "pooled", &global);
    fprintf(outf,"run_status,global,0,0,0,0,0,completed,1,output=%s\n", outpath);
    fclose(outf);
    save_cache_files();
    fprintf(stderr,"all done output=%s pooled_sync_slots=%llu pooled_fail=%llu\n", outpath, (unsigned long long)global.sync_slots, (unsigned long long)global.sync_fail);
    free(rows);
    return 0;
}
