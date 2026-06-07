#include "tau_index.hpp"
#include "tau_binary.hpp"
#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <iostream>
#include <iomanip>
#include <functional>
#include <limits>
#include <string>
#include <thread>
#include <vector>
#include <cstring>

struct Progress5s {
    std::chrono::steady_clock::time_point last;
    Progress5s() : last(std::chrono::steady_clock::now()) {}
    void print(double pct, const char* phase, bool force=false) {
        if (pct < 0.0) pct = 0.0;
        if (pct > 100.0) pct = 100.0;
        auto now = std::chrono::steady_clock::now();
        auto sec = std::chrono::duration_cast<std::chrono::seconds>(now - last).count();
        if (!force && sec < 5) return;
        last = now;
        std::cout << "progress " << std::fixed << std::setprecision(2) << pct << "% " << phase << "\n";
        std::cout.flush();
    }
};

static uint64_t isqrt_u64(uint64_t n) {
    long double d = std::sqrt((long double)n);
    uint64_t r = (uint64_t)d;
    while ((r + 1) != 0 && (r + 1) <= n / (r + 1)) ++r;
    while (r > n / r) --r;
    return r;
}

static bool make_prime_list(uint64_t limit, std::vector<uint32_t>& primes, Progress5s& progress) {
    if (limit > (uint64_t)std::numeric_limits<uint32_t>::max()) return false;
    std::vector<uint8_t> comp((size_t)limit + 1, 0);
    for (uint64_t i = 2; i <= limit; ++i) {
        if ((i & 0xfffffULL) == 0 && limit > 0) progress.print(5.0 * (double)i / (double)limit, "prime-base");
        if (comp[(size_t)i]) continue;
        primes.push_back((uint32_t)i);
        if (i <= limit / i) {
            for (uint64_t m = i * i; m <= limit; ) {
                comp[(size_t)m] = 1;
                if (m > UINT64_MAX - i) break;
                m += i;
            }
        }
    }
    return true;
}

static uint8_t classify_from_omega(uint64_t x, uint8_t omega, uint64_t residual) {
    if (x < 2) return TAU_UNKNOWN;
    if (omega == 0 && residual == x) return TAU_PRIME;
    uint32_t cnt = (uint32_t)omega + (residual > 1 ? 1u : 0u);
    if (cnt == 1) return TAU_PRIME;
    if (cnt == 2) return TAU_SEMIPRIME;
    if (cnt > 2) return TAU_ALMOSTPRIME;
    return TAU_COMPOSITE;
}

static void classify_segment(uint64_t low, size_t begin_i, size_t end_i,
                             const std::vector<uint32_t>& primes,
                             std::vector<uint8_t>& labels) {
    if (begin_i >= end_i) return;
    uint64_t seg_low = low + (uint64_t)begin_i;
    uint64_t seg_high = low + (uint64_t)(end_i - 1);
    size_t n = end_i - begin_i;
    std::vector<uint64_t> rem(n);
    std::vector<uint8_t> omega(n, 0);
    for (size_t i = 0; i < n; ++i) rem[i] = seg_low + (uint64_t)i;

    for (uint32_t p : primes) {
        uint64_t pp = (uint64_t)p;
        if (pp > seg_high / pp && pp > seg_high) break;
        uint64_t start = (seg_low / pp) * pp;
        if (start < seg_low) start += pp;
        for (uint64_t m = start; m <= seg_high; ) {
            size_t idx = (size_t)(m - seg_low);
            while (rem[idx] % pp == 0) {
                rem[idx] /= pp;
                if (omega[idx] < 255) ++omega[idx];
            }
            if (m > UINT64_MAX - pp) break;
            m += pp;
        }
    }

    for (size_t i = 0; i < n; ++i) {
        uint64_t x = seg_low + (uint64_t)i;
        labels[begin_i + i] = classify_from_omega(x, omega[i], rem[i]);
    }
}

int tau_make_index_csv(const char* csv_path, const char* twin_csv_path, const char* out_path,
                       uint64_t L, uint64_t R, unsigned threads, uint64_t block_rows, uint64_t flush_bytes) {
    (void)csv_path;
    (void)twin_csv_path;
    if (L != 1) { std::cerr << "error: make-index range must start at 1\n"; return 2; }
    if (R < 1) { std::cerr << "error: bad upper bound R\n"; return 2; }
    if (R == std::numeric_limits<uint64_t>::max()) { std::cerr << "error: R too large\n"; return 2; }
    if (threads == 0) threads = 1;
    if (threads > 1024) threads = 1024;
    if (block_rows == 0) block_rows = 1u << 20; /* 1 Mi labels */
    if (block_rows < 4096) block_rows = 4096;
    if (flush_bytes == 0) flush_bytes = 256ull * 1024ull * 1024ull; /* safe default: 256 MiB */
    if (flush_bytes < block_rows) flush_bytes = block_rows;

    Progress5s progress;
    progress.print(0.0, "start", true);

    uint64_t root = isqrt_u64(R);
    std::vector<uint32_t> primes;
    progress.print(1.0, "prime-base", true);
    if (!make_prime_list(root, primes, progress)) {
        std::cerr << "error: sqrt(R) is too large for this build\n";
        return 2;
    }
    progress.print(5.0, "prime-base-done", true);

    FILE* f = std::fopen(out_path, "wb");
    if (!f) { std::cerr << "error: cannot open output\n"; return 2; }

    TauBinHeader h{};
    h.magic0 = TAU_BIN_MAGIC0;
    h.magic1 = TAU_BIN_MAGIC1;
    h.version = TAU_BIN_VERSION;
    h.header_size = (uint32_t)sizeof(TauBinHeader);
    h.row_size = 1;
    h.flags = 2; /* dense streaming label table */
    h.range_l = 1;
    h.range_r = R;
    h.count = R;
    h.twin_count = 0;
    h.checksum = 0;
    if (std::fwrite(&h, sizeof(h), 1, f) != 1) {
        std::fclose(f);
        std::cerr << "error: header write failed\n";
        return 2;
    }

    std::vector<uint8_t> labels;
    labels.reserve((size_t)block_rows);
    std::vector<uint8_t> write_buffer;
    size_t flush_cap = flush_bytes > (uint64_t)std::numeric_limits<size_t>::max()
        ? std::numeric_limits<size_t>::max()
        : (size_t)flush_bytes;
    write_buffer.reserve(flush_cap);

    uint64_t written = 0;
    uint64_t flush_count = 0;
    uint64_t twin_count = 0;
    uint8_t cls_x_minus_2 = TAU_UNKNOWN;
    uint8_t cls_x_minus_1 = TAU_UNKNOWN;
    uint64_t checksum = 1469598103934665603ull;

    for (uint64_t low = 1; low <= R; ) {
        uint64_t high = low + block_rows - 1;
        if (high < low || high > R) high = R;
        size_t n = (size_t)(high - low + 1);
        labels.assign(n, TAU_UNKNOWN);

        unsigned use_threads = threads;
        if ((uint64_t)use_threads > (uint64_t)n) use_threads = (unsigned)n;
        if (use_threads < 1) use_threads = 1;

        if (use_threads == 1) {
            classify_segment(low, 0, n, primes, labels);
        } else {
            std::vector<std::thread> workers;
            workers.reserve(use_threads);
            size_t base = n / use_threads;
            size_t extra = n % use_threads;
            size_t pos = 0;
            for (unsigned t = 0; t < use_threads; ++t) {
                size_t len = base + (t < extra ? 1u : 0u);
                size_t begin_i = pos;
                size_t end_i = pos + len;
                pos = end_i;
                workers.emplace_back(classify_segment, low, begin_i, end_i, std::cref(primes), std::ref(labels));
            }
            for (auto& w : workers) w.join();
        }

        for (size_t i = 0; i < n; ++i) {
            uint64_t x = low + (uint64_t)i;
            uint8_t cls = labels[i];
            checksum ^= (uint64_t)cls + 0x9e3779b97f4a7c15ull + (x << 6) + (x >> 2);
            checksum *= 1099511628211ull;
            if (x >= 3 && cls_x_minus_2 == TAU_PRIME && cls == TAU_PRIME) ++twin_count;
            cls_x_minus_2 = cls_x_minus_1;
            cls_x_minus_1 = cls;
        }

        if (n > 0) {
            if (write_buffer.size() + n > write_buffer.capacity()) {
                if (!write_buffer.empty()) {
                    size_t wn = write_buffer.size();
                    if (std::fwrite(write_buffer.data(), 1, wn, f) != wn) {
                        std::fclose(f);
                        std::cerr << "error: label write failed\n";
                        return 2;
                    }
                    write_buffer.clear();
                    ++flush_count;
                }
            }
            if (n > write_buffer.capacity()) {
                if (std::fwrite(labels.data(), 1, n, f) != n) {
                    std::fclose(f);
                    std::cerr << "error: label write failed\n";
                    return 2;
                }
                ++flush_count;
            } else {
                size_t old_size = write_buffer.size();
                write_buffer.resize(old_size + n);
                std::memcpy(write_buffer.data() + old_size, labels.data(), n);
            }
        }
        written += (uint64_t)n;
        double pct = 5.0 + 94.0 * (double)written / (double)R;
        progress.print(pct, "build-buffer");
        if (high == R) break;
        low = high + 1;
    }

    if (!write_buffer.empty()) {
        size_t wn = write_buffer.size();
        if (std::fwrite(write_buffer.data(), 1, wn, f) != wn) {
            std::fclose(f);
            std::cerr << "error: final label write failed\n";
            return 2;
        }
        write_buffer.clear();
        ++flush_count;
    }

    h.twin_count = twin_count;
    h.checksum = checksum;
#if defined(_WIN32)
    _fseeki64(f, 0, SEEK_SET);
#else
    fseeko(f, 0, SEEK_SET);
#endif
    std::fwrite(&h, sizeof(h), 1, f);
    std::fclose(f);

    progress.print(100.0, "done", true);
    std::cout << "index_written " << out_path << "\n";
    std::cout << "range 1 " << R << "\n";
    std::cout << "labels " << R << "\n";
    std::cout << "twins " << twin_count << "\n";
    std::cout << "threads " << threads << "\n";
    std::cout << "block_rows " << block_rows << "\n";
    std::cout << "flush_bytes " << flush_bytes << "\n";
    std::cout << "flush_count " << flush_count << "\n";
    std::cout << "storage dense_buffered_1byte_per_integer\n";
    std::cout << "source internal_tau_sync_sieve_no_miller_rabin_no_external_isprime\n";
    return 0;
}
