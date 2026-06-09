#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <windows.h>

#define INDEX_MAGIC "TSC2CM3"
#define INDEX_HEADER_SIZE 64u
#define INDEX_VERSION 3u
#define RECORD_SIZE 1u

#define DEFAULT_THREADS 4u
#define DEFAULT_CHUNK_MB 1024u
#define MAX_THREADS 64u
#define TWIN_SCAN_BLOCK_BYTES (16u * 1024u * 1024u)
#define PROGRESS_INTERVAL_MS 5000u

#define LABEL_OUTSIDE 0u
#define LABEL_PRIME 1u
#define LABEL_SEMI 2u
#define LABEL_ALMOST 3u
#define LABEL_UNKNOWN 4u

#define RES_C0 1u
#define RES_C2 2u
#define RES_CM 4u
#define RES_PR 8u

typedef struct IndexHeader {
    char magic[8];
    uint32_t version;
    uint32_t record_size;
    uint64_t start;
    uint64_t end;
    uint64_t count;
    uint32_t flags;
    uint32_t reserved0;
    uint64_t reserved1;
    uint64_t reserved2;
    uint64_t reserved3;
} IndexHeader;

typedef struct PrimeList {
    uint32_t *v;
    uint32_t n;
} PrimeList;

typedef struct WorkerTask {
    uint32_t id;
    uint64_t lo;
    uint64_t hi;
    const uint32_t *primes;
    uint32_t prime_count;
    unsigned char *records;
    uint64_t len;
    volatile LONG progress_permille;
    int ok;
    char error[128];
} WorkerTask;


typedef struct ProgressState {
    CRITICAL_SECTION lock;
    uint64_t done;
    uint64_t total;
    volatile LONG stop;
    HANDLE thread;
} ProgressState;


static unsigned char pack_record(unsigned char label, unsigned char resmask) {
    return (unsigned char)((label & 7u) | ((resmask & 15u) << 3));
}

static unsigned char rec_label(unsigned char rec) {
    return (unsigned char)(rec & 7u);
}

static unsigned char rec_resmask(unsigned char rec) {
    return (unsigned char)((rec >> 3) & 15u);
}

static const char *label_name(unsigned char label) {
    switch (label) {
        case LABEL_PRIME: return "P";
        case LABEL_SEMI: return "S";
        case LABEL_ALMOST: return "A";
        case LABEL_OUTSIDE: return "O";
        default: return "U";
    }
}

static const char *label_long_name(unsigned char label) {
    switch (label) {
        case LABEL_PRIME: return "PRIME";
        case LABEL_SEMI: return "SEMIPRIME";
        case LABEL_ALMOST: return "ALMOSTPRIME";
        case LABEL_OUTSIDE: return "OUTSIDE";
        default: return "UNKNOWN";
    }
}

static void print_usage(void) {
    printf("tau_index usage:\n");
    printf("  tau_index build <end> <index.bin> [threads] [chunk_mb]\n");
    printf("  tau_index query <index.bin> <n>\n");
    printf("  tau_index twins <index.bin> <start> <end> [count]\n");
    printf("\nExamples:\n");
    printf("  tau_index build 60000000 index.bin 8 1024\n");
    printf("  tau_index query index.bin 10000\n");
    printf("  tau_index twins index.bin 1 60000000 10\n");
}

static int parse_u64(const char *s, uint64_t *out) {
    char *endp;
    unsigned long long v;

    if (s == NULL || *s == 0) return 0;
    v = strtoull(s, &endp, 10);
    if (*endp != 0) return 0;
    *out = (uint64_t)v;
    return 1;
}

static int parse_u32(const char *s, uint32_t *out) {
    uint64_t v;
    if (!parse_u64(s, &v)) return 0;
    if (v > 0xffffffffULL) return 0;
    *out = (uint32_t)v;
    return 1;
}

static uint64_t isqrt_u64(uint64_t x) {
    uint64_t lo;
    uint64_t hi;
    uint64_t ans;

    lo = 0u;
    hi = 1u;
    while (hi <= x / hi && hi < 0x100000000ULL) {
        hi <<= 1;
    }
    if (hi > 0xffffffffULL) hi = 0xffffffffULL;

    ans = 0u;
    while (lo <= hi) {
        uint64_t mid;
        mid = lo + ((hi - lo) >> 1);
        if (mid == 0u || mid <= x / mid) {
            ans = mid;
            lo = mid + 1u;
        } else {
            if (mid == 0u) break;
            hi = mid - 1u;
        }
    }
    return ans;
}

static int is_endpoint_u64(uint64_t n) {
    uint64_t r;
    if (n <= 3u) return 0;
    r = n % 6u;
    return (r == 1u || r == 5u);
}

static void print_resmask(unsigned char m) {
    int first;
    first = 1;
    if (m & RES_C0) {
        printf("C0");
        first = 0;
    }
    if (m & RES_C2) {
        if (!first) printf("|");
        printf("C2");
        first = 0;
    }
    if (m & RES_CM) {
        if (!first) printf("|");
        printf("CM");
        first = 0;
    }
    if (m & RES_PR) {
        if (!first) printf("|");
        printf("PR");
        first = 0;
    }
    if (first) printf("-");
}

static int seek_u64(FILE *fp, uint64_t off) {
    return _fseeki64(fp, (__int64)off, SEEK_SET) == 0;
}

static void put_u32_le(unsigned char *p, uint32_t v) {
    p[0] = (unsigned char)(v & 255u);
    p[1] = (unsigned char)((v >> 8) & 255u);
    p[2] = (unsigned char)((v >> 16) & 255u);
    p[3] = (unsigned char)((v >> 24) & 255u);
}

static void put_u64_le(unsigned char *p, uint64_t v) {
    p[0] = (unsigned char)(v & 255u);
    p[1] = (unsigned char)((v >> 8) & 255u);
    p[2] = (unsigned char)((v >> 16) & 255u);
    p[3] = (unsigned char)((v >> 24) & 255u);
    p[4] = (unsigned char)((v >> 32) & 255u);
    p[5] = (unsigned char)((v >> 40) & 255u);
    p[6] = (unsigned char)((v >> 48) & 255u);
    p[7] = (unsigned char)((v >> 56) & 255u);
}

static uint32_t get_u32_le(const unsigned char *p) {
    return ((uint32_t)p[0]) |
           ((uint32_t)p[1] << 8) |
           ((uint32_t)p[2] << 16) |
           ((uint32_t)p[3] << 24);
}

static uint64_t get_u64_le(const unsigned char *p) {
    return ((uint64_t)p[0]) |
           ((uint64_t)p[1] << 8) |
           ((uint64_t)p[2] << 16) |
           ((uint64_t)p[3] << 24) |
           ((uint64_t)p[4] << 32) |
           ((uint64_t)p[5] << 40) |
           ((uint64_t)p[6] << 48) |
           ((uint64_t)p[7] << 56);
}

static int write_header(FILE *fp, const IndexHeader *h) {
    unsigned char buf[INDEX_HEADER_SIZE];

    memset(buf, 0, sizeof(buf));

    memcpy(buf + 0u, h->magic, 8u);
    put_u32_le(buf + 8u, h->version);
    put_u32_le(buf + 12u, h->record_size);
    put_u64_le(buf + 16u, h->start);
    put_u64_le(buf + 24u, h->end);
    put_u64_le(buf + 32u, h->count);
    put_u32_le(buf + 40u, h->flags);
    put_u32_le(buf + 44u, h->reserved0);
    put_u64_le(buf + 48u, h->reserved1);
    put_u64_le(buf + 56u, h->reserved2);

    if (!seek_u64(fp, 0u)) return 0;
    return fwrite(buf, 1u, INDEX_HEADER_SIZE, fp) == INDEX_HEADER_SIZE;
}

static int read_header(FILE *fp, IndexHeader *h) {
    unsigned char buf[INDEX_HEADER_SIZE];
    size_t r;

    if (!seek_u64(fp, 0u)) return 0;
    r = fread(buf, 1u, INDEX_HEADER_SIZE, fp);
    if (r != INDEX_HEADER_SIZE) return 0;

    memset(h, 0, sizeof(*h));

    memcpy(h->magic, buf + 0u, 8u);
    h->version = get_u32_le(buf + 8u);
    h->record_size = get_u32_le(buf + 12u);
    h->start = get_u64_le(buf + 16u);
    h->end = get_u64_le(buf + 24u);
    h->count = get_u64_le(buf + 32u);
    h->flags = get_u32_le(buf + 40u);
    h->reserved0 = get_u32_le(buf + 44u);
    h->reserved1 = get_u64_le(buf + 48u);
    h->reserved2 = get_u64_le(buf + 56u);
    h->reserved3 = 0u;

    if (memcmp(h->magic, INDEX_MAGIC, 8u) != 0) return 0;
    if (h->version != INDEX_VERSION) return 0;
    if (h->record_size != RECORD_SIZE) return 0;
    if (h->end < h->start) return 0;
    if (h->count != h->end - h->start + 1u) return 0;

    return 1;
}


static int read_record_raw(FILE *fp, uint64_t index_start, uint64_t n, unsigned char *rec) {
    uint64_t off;

    if (n < index_start) return 0;

    off = (uint64_t)INDEX_HEADER_SIZE + (n - index_start) * (uint64_t)RECORD_SIZE;

    if (!seek_u64(fp, off)) return 0;
    return fread(rec, 1u, 1u, fp) == 1u;
}

static int read_record(FILE *fp, const IndexHeader *h, uint64_t n, unsigned char *rec) {
    if (n < h->start || n > h->end) return 0;
    return read_record_raw(fp, h->start, n, rec);
}

static void worker_set_progress(WorkerTask *t, uint32_t permille) {
    if (permille > 1000u) permille = 1000u;
    InterlockedExchange((volatile LONG*)&t->progress_permille, (LONG)permille);
}

static uint64_t chunk_progress_done(const WorkerTask *tasks, uint32_t tcount) {
    uint32_t i;
    uint64_t done;

    done = 0u;

    for (i = 0u; i < tcount; i++) {
        LONG p;

        if (tasks[i].len == 0u) continue;

        p = tasks[i].progress_permille;
        if (p < 0) p = 0;
        if (p > 1000) p = 1000;

        done += (tasks[i].len * (uint64_t)p) / 1000u;
    }

    return done;
}

static void progress_print(uint64_t done, uint64_t total) {
    double pct;

    if (done > total) done = total;

    pct = 0.0;
    if (total != 0u) {
        pct = 100.0 * (double)done / (double)total;
    }

    printf("[progress] %.2f%% (%llu/%llu)\n",
           pct,
           (unsigned long long)done,
           (unsigned long long)total);
    fflush(stdout);
}

static DWORD WINAPI progress_thread_main(LPVOID arg) {
    ProgressState *ps;
    ULONGLONG last_ms;

    ps = (ProgressState*)arg;
    last_ms = GetTickCount64();

    while (InterlockedCompareExchange(&ps->stop, 0, 0) == 0) {
        ULONGLONG now_ms;

        Sleep(200u);

        now_ms = GetTickCount64();
        if (now_ms - last_ms >= PROGRESS_INTERVAL_MS) {
            uint64_t done;
            uint64_t total;

            EnterCriticalSection(&ps->lock);
            done = ps->done;
            total = ps->total;
            LeaveCriticalSection(&ps->lock);

            progress_print(done, total);
            last_ms = now_ms;
        }
    }

    return 0;
}

static int progress_start(ProgressState *ps, uint64_t total) {
    memset(ps, 0, sizeof(*ps));

    InitializeCriticalSection(&ps->lock);

    ps->done = 0u;
    ps->total = total;
    ps->stop = 0;

    ps->thread = CreateThread(NULL, 0, progress_thread_main, ps, 0, NULL);
    if (ps->thread == NULL) {
        DeleteCriticalSection(&ps->lock);
        return 0;
    }

    return 1;
}

static void progress_set(ProgressState *ps, uint64_t done) {
    EnterCriticalSection(&ps->lock);
    ps->done = done;
    LeaveCriticalSection(&ps->lock);
}

static void progress_stop(ProgressState *ps) {
    InterlockedExchange(&ps->stop, 1);

    if (ps->thread != NULL) {
        WaitForSingleObject(ps->thread, INFINITE);
        CloseHandle(ps->thread);
        ps->thread = NULL;
    }

    progress_print(ps->total, ps->total);

    DeleteCriticalSection(&ps->lock);
}


static int build_base_primes(uint64_t limit64, PrimeList *plist) {
    uint32_t limit;
    uint32_t odd_count;
    unsigned char *comp;
    uint32_t i;
    uint32_t n;
    uint32_t count;
    uint32_t idx;
    if (limit64 > 0xffffffffULL) {
        fprintf(stderr, "sqrt(end) too large for this builder.\n");
        return 0;
    }

    limit = (uint32_t)limit64;
    memset(plist, 0, sizeof(*plist));
    if (limit < 2u) return 1;

    odd_count = limit / 2u + 1u;
    comp = (unsigned char*)calloc((size_t)odd_count, 1u);
    if (comp == NULL) {
        fprintf(stderr, "base prime sieve allocation failed.\n");
        return 0;
    }

    n = 3u;
    while ((uint64_t)n * (uint64_t)n <= (uint64_t)limit) {
        if (!comp[n >> 1]) {
            uint64_t m;
            for (m = (uint64_t)n * (uint64_t)n; m <= (uint64_t)limit; m += (uint64_t)2u * n) {
                comp[(uint32_t)m >> 1] = 1u;
            }
        }
        n += 2u;
    }

    count = 1u;
    for (i = 3u; i <= limit; i += 2u) {
        if (!comp[i >> 1]) count++;
    }

    plist->v = (uint32_t*)malloc((size_t)count * sizeof(uint32_t));
    if (plist->v == NULL) {
        free(comp);
        fprintf(stderr, "base prime list allocation failed.\n");
        return 0;
    }

    idx = 0u;
    plist->v[idx++] = 2u;
    for (i = 3u; i <= limit; i += 2u) {
        if (!comp[i >> 1]) plist->v[idx++] = i;
    }
    plist->n = idx;

    free(comp);
    return 1;
}

static DWORD WINAPI worker_build_lane(LPVOID arg) {
    WorkerTask *t;
    uint64_t len;
    uint8_t *ssat;
    uint8_t *resmask;
    uint32_t *sp;
    uint32_t ip;
    uint64_t mark_total_work;
    uint64_t mark_done_work;

    t = (WorkerTask*)arg;
    t->ok = 0;
    t->error[0] = 0;
    worker_set_progress(t, 0u);

    if (t->hi < t->lo) {
        worker_set_progress(t, 1000u);
        t->ok = 1;
        return 0;
    }

    len = t->hi - t->lo + 1u;
    t->len = len;

    if (len > ((uint64_t)SIZE_MAX / 7u)) {
        strcpy(t->error, "lane too large");
        return 0;
    }

    t->records = (unsigned char*)malloc((size_t)len);
    ssat = (uint8_t*)calloc((size_t)len, 1u);
    resmask = (uint8_t*)calloc((size_t)len, 1u);
    sp = (uint32_t*)calloc((size_t)len, sizeof(uint32_t));

    if (t->records == NULL || ssat == NULL || resmask == NULL || sp == NULL) {
        free(t->records);
        free(ssat);
        free(resmask);
        free(sp);
        strcpy(t->error, "lane allocation failed");
        return 0;
    }

    mark_total_work = 0u;
    for (ip = 0u; ip < t->prime_count; ip++) {
        uint32_t p;
        uint64_t p2;
        uint64_t start;

        p = t->primes[ip];
        if (p < 5u) continue;

        p2 = (uint64_t)p * (uint64_t)p;
        if (p2 > t->hi) break;

        if (p2 > t->lo) {
            start = p2;
        } else {
            start = ((t->lo + (uint64_t)p - 1u) / (uint64_t)p) * (uint64_t)p;
        }

        if (start <= t->hi) {
            mark_total_work += ((t->hi - start) / (uint64_t)p) + 1u;
        }
    }

    if (mark_total_work == 0u) mark_total_work = 1u;
    mark_done_work = 0u;

    for (ip = 0u; ip < t->prime_count; ip++) {
        uint32_t p;
        uint64_t p2;
        uint64_t start;
        uint64_t m;
        uint32_t pm;

        p = t->primes[ip];
        if (p < 5u) continue;

        p2 = (uint64_t)p * (uint64_t)p;
        if (p2 > t->hi) break;

        if ((ip & 63u) == 0u) {
            uint32_t pp;
            if (t->prime_count == 0u) pp = 700u;
            else pp = (uint32_t)(((uint64_t)ip * 700u) / (uint64_t)t->prime_count);
            worker_set_progress(t, pp);
        }

        if (p2 > t->lo) start = p2;
        else start = ((t->lo + (uint64_t)p - 1u) / (uint64_t)p) * (uint64_t)p;

        pm = p % 6u;

        for (m = start; m <= t->hi; m += (uint64_t)p) {
            uint64_t idx;
            uint64_t c;
            uint32_t mr;
            uint32_t cr;
            uint8_t bit;

            mark_done_work++;
            if ((mark_done_work & 0x3ffffu) == 0u) {
                uint32_t pp;
                pp = (uint32_t)((mark_done_work * 700u) / mark_total_work);
                if (pp > 700u) pp = 700u;
                worker_set_progress(t, pp);
            }

            mr = (uint32_t)(m % 6u);
            if (!(mr == 1u || mr == 5u)) continue;

            idx = m - t->lo;
            c = m / (uint64_t)p;
            cr = (uint32_t)(c % 6u);

            if (pm == 1u && cr == 1u) bit = RES_C0;
            else if (pm == 5u && cr == 5u) bit = RES_C2;
            else bit = RES_CM;

            resmask[idx] |= bit;

            if (ssat[idx] == 0u) {
                ssat[idx] = 1u;
                sp[idx] = p;
            } else if (ssat[idx] == 1u && sp[idx] != p) {
                ssat[idx] = 2u;
            }
        }
    }

    worker_set_progress(t, 700u);

    {
        uint64_t i;
        for (i = 0u; i < len; i++) {
            uint64_t x;
            unsigned char label;
            unsigned char rm;

            if ((i & 65535u) == 0u) {
                uint32_t pp;
                pp = 700u + (uint32_t)((i * 300u) / len);
                worker_set_progress(t, pp);
            }

            x = t->lo + i;
            rm = resmask[i];

            if (x < 2u) {
                label = LABEL_OUTSIDE;
            } else if (x == 2u || x == 3u) {
                label = LABEL_PRIME;
            } else if ((x % 2u) == 0u || (x % 3u) == 0u) {
                label = LABEL_UNKNOWN;
            } else if (!is_endpoint_u64(x)) {
                label = LABEL_OUTSIDE;
            } else if (ssat[i] == 0u) {
                label = LABEL_PRIME;
            } else if (ssat[i] >= 2u) {
                label = LABEL_ALMOST;
            } else {
                uint64_t p;
                uint64_t p2;
                p = (uint64_t)sp[i];
                p2 = p * p;
                if (x == p2) label = LABEL_SEMI;
                else if ((x % p2) == 0u) label = LABEL_ALMOST;
                else label = LABEL_SEMI;
            }

            t->records[i] = pack_record(label, rm);
        }
    }

    free(ssat);
    free(resmask);
    free(sp);

    worker_set_progress(t, 1000u);
    t->ok = 1;
    return 0;
}

static unsigned char label_from_omega(uint32_t omega) {
    if (omega == 0u) return LABEL_OUTSIDE;
    if (omega == 1u) return LABEL_PRIME;
    if (omega == 2u) return LABEL_SEMI;
    return LABEL_ALMOST;
}


static int finalize_chunk_labels(FILE *fp,
                                uint64_t index_start,
                                uint64_t chunk_lo,
                                uint64_t chunk_hi,
                                unsigned char *records,
                                uint64_t *stored_u_count) {
    uint64_t i;
    uint64_t len;

    (void)fp;
    (void)index_start;

    len = chunk_hi - chunk_lo + 1u;
    *stored_u_count = 0u;

    for (i = 0u; i < len; i++) {
        uint64_t x;
        uint64_t m;
        uint32_t omega_known;
        unsigned char rec;
        unsigned char label;
        unsigned char rm;

        rec = records[i];
        label = rec_label(rec);
        if (label != LABEL_UNKNOWN) continue;

        x = chunk_lo + i;
        m = x;
        omega_known = 0u;
        rm = rec_resmask(rec);

        while ((m % 2u) == 0u) {
            omega_known++;
            m /= 2u;
            if (omega_known >= 3u) break;
        }

        while (omega_known < 3u && (m % 3u) == 0u) {
            omega_known++;
            m /= 3u;
            if (omega_known >= 3u) break;
        }

        if (omega_known >= 3u) {
            if (rm == 0u) rm = RES_PR;
            records[i] = pack_record(LABEL_ALMOST, rm);
            continue;
        }

        if (m == 1u) {
            unsigned char out_label;
            out_label = label_from_omega(omega_known);
            if ((out_label == LABEL_SEMI || out_label == LABEL_ALMOST) && rm == 0u) {
                rm = RES_PR;
            }
            records[i] = pack_record(out_label, rm);
            continue;
        }

        if (rm == 0u) rm = RES_PR;
        records[i] = pack_record(LABEL_UNKNOWN, rm);
        (*stored_u_count)++;
    }

    return 1;
}


static void print_bytes_gib(const char *name, uint64_t bytes) {
    double gib;
    gib = (double)bytes / 1073741824.0;
    printf("[build] %s=%llu bytes (%.2f GiB)\n",
           name,
           (unsigned long long)bytes,
           gib);
}

static int get_windows_root_for_path(const char *path, char *root, DWORD root_size) {
    char full[MAX_PATH];
    DWORD n;

    if (root_size < 4u) return 0;

    n = GetFullPathNameA(path, MAX_PATH, full, NULL);
    if (n == 0u || n >= MAX_PATH) return 0;

    if (full[0] != 0 && full[1] == ':' && (full[2] == '\\' || full[2] == '/')) {
        root[0] = full[0];
        root[1] = ':';
        root[2] = '\\';
        root[3] = 0;
        return 1;
    }

    return 0;
}

static void print_disk_space_warning(const char *path, uint64_t need_bytes) {
    char root[MAX_PATH];
    ULARGE_INTEGER free_avail;
    ULARGE_INTEGER total_bytes;
    ULARGE_INTEGER total_free;

    if (!get_windows_root_for_path(path, root, MAX_PATH)) {
        printf("[build] disk_free_check=unknown\n");
        return;
    }

    if (!GetDiskFreeSpaceExA(root, &free_avail, &total_bytes, &total_free)) {
        printf("[build] disk_free_check=failed root=%s\n", root);
        return;
    }

    printf("[build] output_root=%s\n", root);
    print_bytes_gib("disk_free_available", (uint64_t)free_avail.QuadPart);

    if ((uint64_t)free_avail.QuadPart < need_bytes) {
        printf("[warning] free disk space is smaller than estimated index size.\n");
    }
}

static void print_open_output_error(const char *path) {
    DWORD winerr;
    winerr = GetLastError();
    fprintf(stderr, "cannot open output file: %s\n", path);
    fprintf(stderr, "errno=%d\n", errno);
    perror("fopen");
    if (winerr != 0u) fprintf(stderr, "GetLastError=%lu\n", (unsigned long)winerr);
}

static int build_index(const char *path, uint64_t end, uint32_t threads, uint32_t chunk_mb) {
    FILE *fp;
    IndexHeader h;
    PrimeList primes;
    uint64_t start;
    uint64_t sqrt_end;
    uint64_t bytes_budget;
    uint64_t nums_per_chunk;
    uint64_t cur;
    uint64_t total_count;
    uint64_t index_bytes;
    uint64_t completed_count;
    uint64_t total_stored_u_count;
    ProgressState progress;
    ULONGLONG t0_ms;
    ULONGLONG t1_ms;

    start = 1u;

    if (end < start) {
        fprintf(stderr, "invalid end\n");
        return 0;
    }

    if (threads == 0u) threads = DEFAULT_THREADS;
    if (threads > MAX_THREADS) threads = MAX_THREADS;
    if (chunk_mb == 0u) chunk_mb = DEFAULT_CHUNK_MB;

    total_count = end - start + 1u;

    if (total_count > UINT64_MAX - (uint64_t)INDEX_HEADER_SIZE) {
        fprintf(stderr, "range too large: index byte size overflows uint64\n");
        return 0;
    }

    index_bytes = (uint64_t)INDEX_HEADER_SIZE + total_count;

    bytes_budget = (uint64_t)chunk_mb * 1024u * 1024u;
    nums_per_chunk = bytes_budget / 8u;
    if (nums_per_chunk < threads) nums_per_chunk = threads;

    sqrt_end = isqrt_u64(end);

    printf("[build] range=1..%llu count=%llu\n",
           (unsigned long long)end,
           (unsigned long long)total_count);
    printf("[build] threads=%u chunk_mb=%u approx_nums_per_chunk=%llu\n",
           threads, chunk_mb, (unsigned long long)nums_per_chunk);
    printf("[build] output=%s\n", path);
    print_bytes_gib("estimated_index_size", index_bytes);
    print_disk_space_warning(path, index_bytes);
    printf("[build] base primes up to sqrt(end)=%llu\n", (unsigned long long)sqrt_end);

    t0_ms = GetTickCount64();
    if (!build_base_primes(sqrt_end, &primes)) return 0;
    t1_ms = GetTickCount64();

    printf("[build] prime_count=%u prime_build_sec=%.3f\n",
           primes.n, (double)(t1_ms - t0_ms) / 1000.0);

    errno = 0;
    SetLastError(0u);
    fp = fopen(path, "w+b");
    if (fp == NULL) {
        print_open_output_error(path);
        free(primes.v);
        return 0;
    }

    memset(&h, 0, sizeof(h));
    memcpy(h.magic, INDEX_MAGIC, 8u);
    h.version = INDEX_VERSION;
    h.record_size = RECORD_SIZE;
    h.start = start;
    h.end = end;
    h.count = total_count;

    if (!write_header(fp, &h)) {
        fprintf(stderr, "cannot write header\n");
        fclose(fp);
        free(primes.v);
        return 0;
    }

    cur = start;
    completed_count = 0u;
    total_stored_u_count = 0u;

    if (!progress_start(&progress, total_count)) {
        fprintf(stderr, "progress thread start failed\n");
        fclose(fp);
        free(primes.v);
        return 0;
    }

    while (cur <= end) {
        uint64_t chunk_lo;
        uint64_t chunk_hi;
        uint64_t chunk_len;
        uint32_t tcount;
        uint32_t ti;
        HANDLE handles[MAX_THREADS];
        WorkerTask tasks[MAX_THREADS];
        uint64_t base_len;
        uint64_t rem;
        uint64_t lane_lo;
        uint64_t stored_u_in_chunk;

        chunk_lo = cur;
        if (end - cur + 1u > nums_per_chunk) chunk_hi = cur + nums_per_chunk - 1u;
        else chunk_hi = end;
        chunk_len = chunk_hi - chunk_lo + 1u;

        tcount = threads;
        if ((uint64_t)tcount > chunk_len) tcount = (uint32_t)chunk_len;
        if (tcount == 0u) tcount = 1u;

        base_len = chunk_len / (uint64_t)tcount;
        rem = chunk_len % (uint64_t)tcount;
        lane_lo = chunk_lo;

        memset(tasks, 0, sizeof(tasks));
        memset(handles, 0, sizeof(handles));

        for (ti = 0u; ti < tcount; ti++) {
            uint64_t lane_len;
            lane_len = base_len + ((uint64_t)ti < rem ? 1u : 0u);
            tasks[ti].id = ti;
            tasks[ti].lo = lane_lo;
            tasks[ti].hi = lane_lo + lane_len - 1u;
            tasks[ti].primes = primes.v;
            tasks[ti].prime_count = primes.n;
            handles[ti] = CreateThread(NULL, 0, worker_build_lane, &tasks[ti], 0, NULL);
            if (handles[ti] == NULL) {
                fprintf(stderr, "CreateThread failed\n");
                progress_stop(&progress);
                fclose(fp);
                free(primes.v);
                return 0;
            }
            lane_lo += lane_len;
        }

        for (;;) {
            DWORD wr;

            wr = WaitForMultipleObjects(tcount, handles, TRUE, 1000u);

            progress_set(&progress, completed_count + chunk_progress_done(tasks, tcount));

            if (wr == WAIT_OBJECT_0) break;
            if (wr == WAIT_TIMEOUT) continue;

            fprintf(stderr, "WaitForMultipleObjects failed\n");
            progress_stop(&progress);
            fclose(fp);
            free(primes.v);
            return 0;
        }

        for (ti = 0u; ti < tcount; ti++) {
            CloseHandle(handles[ti]);
            if (!tasks[ti].ok) {
                fprintf(stderr, "worker %u failed: %s\n", ti, tasks[ti].error);
                progress_stop(&progress);
                fclose(fp);
                free(primes.v);
                return 0;
            }
        }

        stored_u_in_chunk = 0u;

        for (ti = 0u; ti < tcount; ti++) {
            uint64_t lane_stored_u;
            uint64_t write_off;

            lane_stored_u = 0u;

            if (!finalize_chunk_labels(fp,
                                       start,
                                       tasks[ti].lo,
                                       tasks[ti].hi,
                                       tasks[ti].records,
                                       &lane_stored_u)) {
                fprintf(stderr, "finalize_chunk_labels failed\n");
                free(tasks[ti].records);
                progress_stop(&progress);
                fclose(fp);
                free(primes.v);
                return 0;
            }

            stored_u_in_chunk += lane_stored_u;

            write_off = (uint64_t)INDEX_HEADER_SIZE +
                        (tasks[ti].lo - start) * (uint64_t)RECORD_SIZE;

            clearerr(fp);

            if (!seek_u64(fp, write_off)) {
                fprintf(stderr, "seek before write failed at offset %llu\n",
                        (unsigned long long)write_off);
                free(tasks[ti].records);
                progress_stop(&progress);
                fclose(fp);
                free(primes.v);
                return 0;
            }

            if (fwrite(tasks[ti].records, 1u, (size_t)tasks[ti].len, fp) != (size_t)tasks[ti].len) {
                fprintf(stderr, "write failed at lane %llu..%llu\n",
                        (unsigned long long)tasks[ti].lo,
                        (unsigned long long)tasks[ti].hi);
                free(tasks[ti].records);
                progress_stop(&progress);
                fclose(fp);
                free(primes.v);
                return 0;
            }

            free(tasks[ti].records);
            tasks[ti].records = NULL;
        }

        total_stored_u_count += stored_u_in_chunk;

        fflush(fp);

        completed_count += chunk_len;
        progress_set(&progress, completed_count);

        if (chunk_hi == end) break;
        cur = chunk_hi + 1u;
    }

    progress_set(&progress, total_count);
    progress_stop(&progress);

    fclose(fp);
    free(primes.v);

    printf("[build] stored_u_count=%llu\n", (unsigned long long)total_stored_u_count);
    printf("[build] done: %s\n", path);
    return 1;
}

static unsigned char classify_query_full(FILE *fp, const IndexHeader *h, uint64_t n, unsigned char *out_resmask) {
    uint64_t m;
    uint32_t omega_known;
    unsigned char rec;
    unsigned char label;
    unsigned char rm;

    if (n < 2u) {
        *out_resmask = 0u;
        return LABEL_OUTSIDE;
    }

    m = n;
    omega_known = 0u;

    while ((m % 2u) == 0u) {
        omega_known++;
        m /= 2u;
        if (omega_known >= 3u) {
            *out_resmask = RES_PR;
            return LABEL_ALMOST;
        }
    }

    while ((m % 3u) == 0u) {
        omega_known++;
        m /= 3u;
        if (omega_known >= 3u) {
            *out_resmask = RES_PR;
            return LABEL_ALMOST;
        }
    }

    if (m == 1u) {
        *out_resmask = omega_known >= 2u ? RES_PR : 0u;
        return label_from_omega(omega_known);
    }

    if (m < h->start || m > h->end) {
        *out_resmask = RES_PR;
        return LABEL_UNKNOWN;
    }

    if (!read_record(fp, h, m, &rec)) {
        *out_resmask = RES_PR;
        return LABEL_UNKNOWN;
    }

    label = rec_label(rec);
    rm = rec_resmask(rec);
    if (rm == 0u && omega_known > 0u) rm = RES_PR;

    if (label == LABEL_PRIME) {
        omega_known += 1u;
    } else if (label == LABEL_SEMI) {
        omega_known += 2u;
    } else if (label == LABEL_ALMOST) {
        *out_resmask = rm;
        return LABEL_ALMOST;
    } else {
        *out_resmask = rm;
        return LABEL_UNKNOWN;
    }

    *out_resmask = rm;
    return label_from_omega(omega_known);
}


static int cmd_query(const char *path, uint64_t n) {
    FILE *fp;
    IndexHeader h;
    unsigned char rec;
    unsigned char stored_label;
    unsigned char stored_resmask;
    unsigned char label;
    unsigned char resmask;

    fp = fopen(path, "rb");
    if (fp == NULL) {
        fprintf(stderr, "cannot open index\n");
        return 0;
    }

    if (!read_header(fp, &h)) {
        fprintf(stderr, "bad index header\n");
        fclose(fp);
        return 0;
    }

    if (!read_record(fp, &h, n, &rec)) {
        printf("n=%llu outside index range %llu..%llu\n",
               (unsigned long long)n,
               (unsigned long long)h.start,
               (unsigned long long)h.end);
        fclose(fp);
        return 1;
    }

    stored_label = rec_label(rec);
    stored_resmask = rec_resmask(rec);
    label = classify_query_full(fp, &h, n, &resmask);

    printf("n=%llu\n", (unsigned long long)n);
    printf("range=%llu..%llu\n", (unsigned long long)h.start, (unsigned long long)h.end);
    printf("label=%s\n", label_name(label));
    printf("label_long=%s\n", label_long_name(label));
    if (label == LABEL_SEMI) printf("meaning=Omega(n)=2\n");
    else if (label == LABEL_ALMOST) printf("meaning=Omega(n)>=3\n");
    else if (label == LABEL_PRIME) printf("meaning=prime\n");
    printf("resmask=");
    print_resmask(resmask);
    printf("\n");
    printf("stored_label=%s\n", label_name(stored_label));
    printf("stored_resmask=");
    print_resmask(stored_resmask);
    printf("\n");

    fclose(fp);
    return 1;
}


static int cmd_twins(const char *path, uint64_t req_start, uint64_t req_end, uint32_t want_count) {
    FILE *fp;
    IndexHeader h;
    uint64_t lo;
    uint64_t hi;
    uint32_t found;
    unsigned char *buf;

    fp = fopen(path, "rb");
    if (fp == NULL) {
        fprintf(stderr, "cannot open index\n");
        return 0;
    }

    if (!read_header(fp, &h)) {
        fprintf(stderr, "bad index header\n");
        fclose(fp);
        return 0;
    }

    if (want_count == 0u) want_count = 10u;

    lo = req_start;
    hi = req_end;

    if (lo < h.start) lo = h.start;
    if (hi > h.end) hi = h.end;

    if (hi < lo) {
        printf("empty search range\n");
        fclose(fp);
        return 1;
    }

    if (hi < 3u) {
        printf("no twin primes\n");
        fclose(fp);
        return 1;
    }

    buf = (unsigned char*)malloc((size_t)TWIN_SCAN_BLOCK_BYTES);
    if (buf == NULL) {
        fprintf(stderr, "twin scan buffer allocation failed\n");
        fclose(fp);
        return 0;
    }

    found = 0u;
    printf("largest twin primes in %llu..%llu\n",
           (unsigned long long)lo,
           (unsigned long long)hi);

    {
        uint64_t block_hi;

        block_hi = hi;

        while (block_hi >= lo && found < want_count) {
            uint64_t block_lo;
            uint64_t block_len;
            uint64_t p;

            if (block_hi - lo + 1u > (uint64_t)TWIN_SCAN_BLOCK_BYTES) {
                block_lo = block_hi - (uint64_t)TWIN_SCAN_BLOCK_BYTES + 1u;
            } else {
                block_lo = lo;
            }

            if (block_lo > h.start && block_lo >= 2u) {
                block_lo -= 2u;
            }

            block_len = block_hi - block_lo + 1u;

            if (!seek_u64(fp, (uint64_t)INDEX_HEADER_SIZE + (block_lo - h.start))) {
                fprintf(stderr, "twin scan seek failed\n");
                free(buf);
                fclose(fp);
                return 0;
            }

            if (fread(buf, 1u, (size_t)block_len, fp) != (size_t)block_len) {
                fprintf(stderr, "twin scan read failed\n");
                free(buf);
                fclose(fp);
                return 0;
            }

            if (block_hi >= 2u) p = block_hi - 2u;
            else break;

            while (p >= block_lo && p >= lo && found < want_count) {
                uint64_t i1;
                uint64_t i2;

                i1 = p - block_lo;
                i2 = i1 + 2u;

                if (i2 < block_len &&
                    rec_label(buf[i1]) == LABEL_PRIME &&
                    rec_label(buf[i2]) == LABEL_PRIME) {
                    printf("%u: %llu %llu\n",
                           found + 1u,
                           (unsigned long long)p,
                           (unsigned long long)(p + 2u));
                    found++;
                }

                if (p == 0u) break;
                p--;
            }

            if (block_lo <= lo) break;
            block_hi = block_lo - 1u;
        }
    }

    if (found == 0u) printf("none\n");

    free(buf);
    fclose(fp);
    return 1;
}


int main(int argc, char **argv) {
    if (argc < 2) {
        print_usage();
        return 1;
    }

    if (strcmp(argv[1], "build") == 0) {
        uint64_t end;
        uint32_t threads;
        uint32_t chunk_mb;

        if (argc < 4) {
            print_usage();
            return 1;
        }

        if (!parse_u64(argv[2], &end)) {
            fprintf(stderr, "bad end\n");
            return 1;
        }

        threads = DEFAULT_THREADS;
        chunk_mb = DEFAULT_CHUNK_MB;

        if (argc >= 5) {
            if (!parse_u32(argv[4], &threads)) {
                fprintf(stderr, "bad threads\n");
                return 1;
            }
        }

        if (argc >= 6) {
            if (!parse_u32(argv[5], &chunk_mb)) {
                fprintf(stderr, "bad chunk_mb\n");
                return 1;
            }
        }

        return build_index(argv[3], end, threads, chunk_mb) ? 0 : 2;
    }

    if (strcmp(argv[1], "query") == 0) {
        uint64_t n;

        if (argc < 4) {
            print_usage();
            return 1;
        }

        if (!parse_u64(argv[3], &n)) {
            fprintf(stderr, "bad n\n");
            return 1;
        }

        return cmd_query(argv[2], n) ? 0 : 2;
    }

    if (strcmp(argv[1], "twins") == 0) {
        uint64_t start;
        uint64_t end;
        uint32_t count;

        if (argc < 5) {
            print_usage();
            return 1;
        }

        if (!parse_u64(argv[3], &start) || !parse_u64(argv[4], &end)) {
            fprintf(stderr, "bad start/end\n");
            return 1;
        }

        count = 10u;
        if (argc >= 6) {
            if (!parse_u32(argv[5], &count)) {
                fprintf(stderr, "bad count\n");
                return 1;
            }
        }

        return cmd_twins(argv[2], start, end, count) ? 0 : 2;
    }

    print_usage();
    return 1;
}
