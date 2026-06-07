#include "tau_twin.hpp"
#include "tau_binary.hpp"
#include <algorithm>
#include <chrono>
#include <cstdio>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

int tau_twin_top10(const char* index_path) {
    FILE* f = nullptr;
    TauBinHeader h{};
    std::string err;
    if (!tau_open_index(index_path, &f, h, err)) { std::cerr << "error: " << err << "\n"; return 2; }

    auto last = std::chrono::steady_clock::now();
    auto print_progress = [&](double pct, const char* phase, bool force=false) {
        if (pct < 0.0) pct = 0.0;
        if (pct > 100.0) pct = 100.0;
        auto now = std::chrono::steady_clock::now();
        auto sec = std::chrono::duration_cast<std::chrono::seconds>(now - last).count();
        if (!force && sec < 5) return;
        last = now;
        std::cout << "progress " << std::fixed << std::setprecision(2) << pct << "% " << phase << "\n";
        std::cout.flush();
    };

    std::cout << "index_range " << h.range_l << " " << h.range_r << "\n";
    print_progress(0.0, "start", true);

    const uint64_t chunk = 1u << 20;
    std::vector<uint8_t> buf;
    int printed = 0;
    uint64_t scanned = 0;

    if (h.range_r < 3) {
        print_progress(100.0, "done", true);
        std::cout << "none\n";
        std::fclose(f);
        return 0;
    }

    uint64_t end = h.range_r;
    while (end >= 3 && printed < 10) {
        uint64_t begin = (end > chunk ? end - chunk + 1 : 1);
        if (begin < 1) begin = 1;
        size_t n = (size_t)(end - begin + 1);
        buf.resize(n);
        uint64_t off = tau_label_offset(h, begin);
#if defined(_WIN32)
        if (_fseeki64(f, (long long)off, SEEK_SET) != 0) { std::cerr << "error: seek failed\n"; std::fclose(f); return 2; }
#else
        if (fseeko(f, (off_t)off, SEEK_SET) != 0) { std::cerr << "error: seek failed\n"; std::fclose(f); return 2; }
#endif
        if (std::fread(buf.data(), 1, n, f) != n) { std::cerr << "error: read failed\n"; std::fclose(f); return 2; }

        for (uint64_t x = (end >= 2 ? end - 2 : 0); x >= begin && printed < 10; --x) {
            uint8_t a = buf[(size_t)(x - begin)];
            uint8_t b;
            if (x + 2 <= end) b = buf[(size_t)(x + 2 - begin)];
            else {
                if (!tau_read_label_at(f, h, x + 2, b, err)) { std::cerr << "error: " << err << "\n"; std::fclose(f); return 2; }
            }
            if (a == TAU_PRIME && b == TAU_PRIME) {
                std::cout << x << " " << (x + 2) << "\n";
                ++printed;
            }
            if (x == begin) break;
        }

        scanned += n;
        print_progress(100.0 * (double)scanned / (double)h.count, "scan-index");
        if (begin == 1) break;
        end = begin - 1;
    }

    print_progress(100.0, "done", true);
    if (printed == 0) std::cout << "none\n";
    std::fclose(f);
    return 0;
}
