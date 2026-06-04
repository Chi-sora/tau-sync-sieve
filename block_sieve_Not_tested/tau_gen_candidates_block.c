/*
 * tau_gen_candidates_block.c
 *
 * Tau-sync candidate generator: block-sieve optimized version.
 *
 * Purpose:
 *   Generate twin-prime candidates of the form:
 *
 *       p0 = 6*k - 1
 *       p1 = 6*k + 1
 *
 *   without GMP, without factorization, and without constructing p0/p1.
 *
 * Method:
 *   For each small prime r > 3:
 *
 *       p0 divisible by r  <=>  6*k - 1 == 0 mod r
 *       p1 divisible by r  <=>  6*k + 1 == 0 mod r
 *
 *   Since gcd(6,r)=1, inv6 exists.
 *
 *       bad_minus = inv6 mod r
 *       bad_plus  = -inv6 mod r
 *
 *   A k is rejected if:
 *
 *       k == bad_minus mod r
 *       or
 *       k == bad_plus mod r
 *
 * Optimization:
 *   Old style:
 *       update/check residues for every k and every prime.
 *
 *   Block-sieve style:
 *       for each block, mark bad offsets by each prime.
 *       update base residues once per block.
 *
 * Build:
 *   gcc -O3 -std=c11 -Wall -Wextra -march=native -o tau_gen_candidates_block.exe src/tau_gen_candidates_block.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

typedef struct {
    uint32_t r;
    uint32_t bad_minus;
    uint32_t bad_plus;
    uint32_t rem_base;
} Row;

typedef struct {
    unsigned long long steps;
    unsigned int block_size;
    unsigned long long progress_every_blocks;
    int filter_primes;
    int resume;
    int emit_enabled;
    char emit_path[512];
    char checkpoint_path[512];
    char log_path[512];
} Config;

static FILE *g_log = NULL;

static void log_line(const char *s) {
    printf("%s\n", s);
    fflush(stdout);
    if (g_log) {
        fprintf(g_log, "%s\n", s);
        fflush(g_log);
    }
}

static void die(const char *s) {
    fprintf(stderr, "[error] %s\n", s);
    if (g_log) {
        fprintf(g_log, "ERROR %s\n", s);
        fflush(g_log);
    }
    exit(1);
}

static int eqs(const char *a, const char *b) {
    return strcmp(a, b) == 0;
}

static void defaults(Config *c) {
    c->steps = 100000000ULL;
    c->block_size = 1048576U;
    c->progress_every_blocks = 1ULL;
    c->filter_primes = 10000;
    c->resume = 0;
    c->emit_enabled = 1;
    strcpy(c->emit_path, "out\\candidates_block.csv");
    strcpy(c->checkpoint_path, "checkpoint\\block_state.txt");
    strcpy(c->log_path, "logs\\block.log");
}

static void parse(Config *c, int argc, char **argv) {
    for (int i=1;i<argc;i++) {
        if (eqs(argv[i],"--steps") && i+1<argc) {
            c->steps = strtoull(argv[++i], NULL, 10);
        } else if (eqs(argv[i],"--block-size") && i+1<argc) {
            c->block_size = (unsigned int)strtoul(argv[++i], NULL, 10);
        } else if (eqs(argv[i],"--filter-primes") && i+1<argc) {
            c->filter_primes = atoi(argv[++i]);
        } else if (eqs(argv[i],"--progress-every-blocks") && i+1<argc) {
            c->progress_every_blocks = strtoull(argv[++i], NULL, 10);
        } else if (eqs(argv[i],"--emit") && i+1<argc) {
            strncpy(c->emit_path, argv[++i], sizeof(c->emit_path)-1);
            c->emit_path[sizeof(c->emit_path)-1] = 0;
            c->emit_enabled = 1;
        } else if (eqs(argv[i],"--checkpoint") && i+1<argc) {
            strncpy(c->checkpoint_path, argv[++i], sizeof(c->checkpoint_path)-1);
            c->checkpoint_path[sizeof(c->checkpoint_path)-1] = 0;
        } else if (eqs(argv[i],"--log") && i+1<argc) {
            strncpy(c->log_path, argv[++i], sizeof(c->log_path)-1);
            c->log_path[sizeof(c->log_path)-1] = 0;
        } else if (eqs(argv[i],"--resume")) {
            c->resume = 1;
        } else if (eqs(argv[i],"--no-emit")) {
            c->emit_enabled = 0;
        } else if (eqs(argv[i],"--help")) {
            printf("usage:\n");
            printf("  tau_gen_candidates_block.exe --steps N --filter-primes M --block-size B\n");
            printf("  tau_gen_candidates_block.exe --resume --steps N --filter-primes M --block-size B\n");
            printf("  tau_gen_candidates_block.exe --no-emit --steps N --filter-primes M\n");
            exit(0);
        } else {
            die("bad arg");
        }
    }
    if (c->steps == 0) die("--steps must be positive");
    if (c->block_size == 0) die("--block-size must be positive");
    if (c->filter_primes < 1) c->filter_primes = 1;
    if (c->progress_every_blocks == 0) c->progress_every_blocks = 1;
}

static int is_prime_ui(uint32_t n) {
    if (n < 2) return 0;
    if ((n % 2) == 0) return n == 2;
    for (uint32_t d=3; (uint64_t)d*d <= n; d+=2) {
        if ((n % d) == 0) return 0;
    }
    return 1;
}

static uint32_t modinv_ui(uint32_t a, uint32_t m) {
    int64_t t=0, nt=1;
    int64_t r=m, nr=a % m;
    while (nr != 0) {
        int64_t q = r / nr;
        int64_t tmp = t - q*nt;
        t = nt;
        nt = tmp;
        tmp = r - q*nr;
        r = nr;
        nr = tmp;
    }
    if (r > 1) return 0;
    if (t < 0) t += m;
    return (uint32_t)t;
}

static Row *build_rows(int want, int *out_n) {
    Row *rows = (Row*)calloc((size_t)want, sizeof(Row));
    if (!rows) die("row alloc failed");

    int n = 0;
    for (uint32_t r=5; n<want; r+=2) {
        if (!is_prime_ui(r)) continue;
        uint32_t inv6 = modinv_ui(6, r);
        if (!inv6) continue;
        rows[n].r = r;
        rows[n].bad_minus = inv6 % r;
        rows[n].bad_plus = (r - inv6) % r;
        rows[n].rem_base = 1 % r;
        n++;
    }
    *out_n = n;
    return rows;
}

static void dec_add_ull(char **ps, size_t *pcap, unsigned long long add) {
    if (add == 0) return;

    char addbuf[32];
    snprintf(addbuf, sizeof(addbuf), "%llu", add);

    char *s = *ps;
    size_t len = strlen(s);
    size_t alen = strlen(addbuf);
    size_t max = len > alen ? len : alen;

    if (max + 3 > *pcap) {
        *pcap = (max + 3) * 2;
        s = (char*)realloc(s, *pcap);
        if (!s) die("decimal realloc failed");
        *ps = s;
    }

    char *tmp = (char*)calloc(max + 3, 1);
    if (!tmp) die("decimal temp alloc failed");

    int carry = 0;
    size_t outpos = 0;

    for (size_t i=0; i<max; i++) {
        int da = 0;
        int db = 0;

        if (i < len) da = s[len - 1 - i] - '0';
        if (i < alen) db = addbuf[alen - 1 - i] - '0';

        int sum = da + db + carry;
        tmp[outpos++] = (char)('0' + (sum % 10));
        carry = sum / 10;
    }

    while (carry) {
        tmp[outpos++] = (char)('0' + (carry % 10));
        carry /= 10;
    }

    for (size_t i=0; i<outpos; i++) {
        s[i] = tmp[outpos - 1 - i];
    }
    s[outpos] = 0;

    free(tmp);
}

static char *dec_add_ull_copy(const char *base, unsigned long long add) {
    size_t cap = strlen(base) + 64;
    char *s = (char*)malloc(cap);
    if (!s) die("decimal copy alloc failed");
    strcpy(s, base);
    dec_add_ull(&s, &cap, add);
    return s;
}

static int file_exists_nonempty(const char *path) {
    FILE *fp = fopen(path, "rb");
    if (!fp) return 0;
    fseek(fp, 0, SEEK_END);
    long size = ftell(fp);
    fclose(fp);
    return size > 0;
}

static int save_checkpoint(const char *path, const char *base_k, const Row *rows, int n) {
    FILE *fp = fopen(path, "w");
    if (!fp) return 0;

    fprintf(fp, "k=%s\n", base_k);
    fprintf(fp, "rows=%d\n", n);
    fprintf(fp, "residues=");
    for (int i=0; i<n; i++) {
        if (i) fprintf(fp, ",");
        fprintf(fp, "%u", rows[i].rem_base);
    }
    fprintf(fp, "\n");

    fclose(fp);
    return 1;
}

static int load_checkpoint(const char *path, char **pk, size_t *pcap, Row *rows, int n) {
    FILE *fp = fopen(path, "r");
    if (!fp) return 0;

    char buf[1048576];

    if (!fgets(buf, sizeof(buf), fp)) {
        fclose(fp);
        return 0;
    }
    if (strncmp(buf, "k=", 2) != 0) {
        fclose(fp);
        return 0;
    }

    char *kstr = buf + 2;
    kstr[strcspn(kstr, "\r\n")] = 0;
    size_t need = strlen(kstr) + 64;
    if (need > *pcap) {
        *pcap = need;
        *pk = (char*)realloc(*pk, *pcap);
        if (!*pk) die("checkpoint decimal realloc failed");
    }
    strcpy(*pk, kstr);

    if (!fgets(buf, sizeof(buf), fp)) {
        fclose(fp);
        return 0;
    }

    if (!fgets(buf, sizeof(buf), fp)) {
        fclose(fp);
        return 0;
    }
    if (strncmp(buf, "residues=", 9) != 0) {
        fclose(fp);
        return 0;
    }

    char *p = buf + 9;
    for (int i=0; i<n; i++) {
        rows[i].rem_base = (uint32_t)strtoul(p, &p, 10);
        if (*p == ',') p++;
        else if (i + 1 < n) {
            fclose(fp);
            return 0;
        }
    }

    fclose(fp);
    return 1;
}

static void mark_bad_offsets(unsigned char *bad, unsigned int block_len, const Row *rows, int n) {
    memset(bad, 0, (size_t)block_len);

    for (int i=0; i<n; i++) {
        uint32_t r = rows[i].r;
        uint32_t rem = rows[i].rem_base;

        uint32_t first1 = rows[i].bad_minus >= rem ? rows[i].bad_minus - rem : rows[i].bad_minus + r - rem;
        uint32_t first2 = rows[i].bad_plus  >= rem ? rows[i].bad_plus  - rem : rows[i].bad_plus  + r - rem;

        for (uint32_t pos = first1; pos < block_len; pos += r) {
            bad[pos] = 1;
        }

        if (first2 != first1) {
            for (uint32_t pos = first2; pos < block_len; pos += r) {
                bad[pos] = 1;
            }
        }
    }
}

static void advance_base_residues(Row *rows, int n, unsigned int block_len) {
    for (int i=0; i<n; i++) {
        rows[i].rem_base = (rows[i].rem_base + (block_len % rows[i].r)) % rows[i].r;
    }
}

int main(int argc, char **argv) {
    Config cfg;
    defaults(&cfg);
    parse(&cfg, argc, argv);

    g_log = fopen(cfg.log_path, "a");

    char line[512];
    snprintf(line, sizeof(line),
        "RUN block_sieve steps=%llu filter_primes=%d block_size=%u emit=%d",
        cfg.steps, cfg.filter_primes, cfg.block_size, cfg.emit_enabled);
    log_line(line);
    log_line("INFO no GMP");
    log_line("INFO no factorization");
    log_line("INFO no per-step residue update");
    log_line("INFO block marking by small-prime residues");

    int nrows = 0;
    Row *rows = build_rows(cfg.filter_primes, &nrows);

    size_t kcap = 128;
    char *base_k = (char*)calloc(kcap, 1);
    if (!base_k) die("base_k alloc failed");
    strcpy(base_k, "1");

    if (cfg.resume) {
        if (!load_checkpoint(cfg.checkpoint_path, &base_k, &kcap, rows, nrows)) {
            die("resume requested but checkpoint not readable");
        }
        log_line("RESUME loaded checkpoint");
    }

    unsigned char *bad = (unsigned char*)malloc((size_t)cfg.block_size);
    if (!bad) die("block allocation failed");

    FILE *emit_fp = NULL;
    char *emit_buf = NULL;

    if (cfg.emit_enabled) {
        int exists = file_exists_nonempty(cfg.emit_path);
        emit_fp = fopen(cfg.emit_path, "a");
        if (!emit_fp) die("cannot open emit CSV");

        emit_buf = (char*)malloc(8 * 1024 * 1024);
        if (emit_buf) setvbuf(emit_fp, emit_buf, _IOFBF, 8 * 1024 * 1024);

        if (!exists) {
            fprintf(emit_fp, "k,k_digits,filter_primes\n");
        }
    }

    unsigned long long processed = 0;
    unsigned long long emitted = 0;
    unsigned long long filtered = 0;
    unsigned long long block_id = 0;

    clock_t t0 = clock();

    while (processed < cfg.steps) {
        unsigned long long remain = cfg.steps - processed;
        unsigned int block_len = remain < cfg.block_size ? (unsigned int)remain : cfg.block_size;

        mark_bad_offsets(bad, block_len, rows, nrows);

        for (unsigned int off=0; off<block_len; off++) {
            if (!bad[off]) {
                emitted++;
                if (emit_fp) {
                    char *kstr = dec_add_ull_copy(base_k, off);
                    fprintf(emit_fp, "%s,%zu,%d\n", kstr, strlen(kstr), nrows);
                    free(kstr);
                }
            } else {
                filtered++;
            }
        }

        dec_add_ull(&base_k, &kcap, block_len);
        advance_base_residues(rows, nrows, block_len);

        processed += block_len;
        block_id++;

        if ((block_id % cfg.progress_every_blocks) == 0ULL) {
            if (emit_fp) fflush(emit_fp);
            save_checkpoint(cfg.checkpoint_path, base_k, rows, nrows);

            double sec = (double)(clock() - t0) / (double)CLOCKS_PER_SEC;
            double rate = sec > 0.0 ? (double)processed / sec : 0.0;

            snprintf(line, sizeof(line),
                "PROGRESS processed=%llu k_digits=%zu emitted=%llu filtered=%llu rate_steps_per_sec=%.2f",
                processed, strlen(base_k), emitted, filtered, rate);
            log_line(line);
        }
    }

    if (emit_fp) fflush(emit_fp);
    save_checkpoint(cfg.checkpoint_path, base_k, rows, nrows);

    double sec = (double)(clock() - t0) / (double)CLOCKS_PER_SEC;
    double rate = sec > 0.0 ? (double)processed / sec : 0.0;

    snprintf(line, sizeof(line),
        "DONE processed=%llu k_digits=%zu emitted=%llu filtered=%llu rate_steps_per_sec=%.2f",
        processed, strlen(base_k), emitted, filtered, rate);
    log_line(line);

    if (emit_fp) fclose(emit_fp);
    if (emit_buf) free(emit_buf);
    free(bad);
    free(base_k);
    free(rows);
    if (g_log) fclose(g_log);
    return 0;
}
