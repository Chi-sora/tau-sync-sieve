#include "tau_index.hpp"
#include "tau_query.hpp"
#include "tau_twin.hpp"
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>
#include <thread>

static bool parse_u64(const char* s, uint64_t& out) {
    if (!s || !*s) return false;
    uint64_t v = 0;
    for (const char* p = s; *p; ++p) {
        if (*p < '0' || *p > '9') return false;
        uint64_t d = (uint64_t)(*p - '0');
        if (v > (UINT64_MAX - d) / 10) return false;
        v = v * 10 + d;
    }
    out = v;
    return true;
}

static unsigned parse_threads(int argc, char** argv, unsigned defv) {
    for (int i = 1; i + 1 < argc; ++i) {
        if (std::strcmp(argv[i], "--threads") == 0) {
            uint64_t v = 0;
            if (parse_u64(argv[i + 1], v) && v > 0 && v <= 1024) return (unsigned)v;
        }
    }
    return defv;
}

static uint64_t parse_block_rows(int argc, char** argv, uint64_t defv) {
    for (int i = 1; i + 1 < argc; ++i) {
        if (std::strcmp(argv[i], "--block-rows") == 0) {
            uint64_t v = 0;
            if (parse_u64(argv[i + 1], v) && v > 0) return v;
        }
    }
    return defv;
}

static uint64_t parse_flush_bytes(int argc, char** argv, uint64_t defv) {
    for (int i = 1; i + 1 < argc; ++i) {
        if (std::strcmp(argv[i], "--flush-bytes") == 0) {
            uint64_t v = 0;
            if (parse_u64(argv[i + 1], v) && v > 0) return v;
        }
        if (std::strcmp(argv[i], "--flush-mib") == 0) {
            uint64_t v = 0;
            if (parse_u64(argv[i + 1], v) && v > 0 && v <= UINT64_MAX / 1048576ull) return v * 1048576ull;
        }
    }
    return defv;
}

static void usage() {
    std::cout << "tausync commands:\n";
    std::cout << "  tausync.exe make-index R out.tsb [--threads N] [--block-rows N] [--flush-mib N]\n";
    std::cout << "  tausync.exe query index.tsb X\n";
    std::cout << "  tausync.exe label index.tsb X\n";
    std::cout << "  tausync.exe label index.tsb\n";
    std::cout << "  tausync.exe twin-top10 index.tsb\n";
    std::cout << "policy:\n";
    std::cout << "  make-index always builds [1, R]\n";
    std::cout << "  hit  -> stored class\n";
    std::cout << "  miss -> unknown\n";
    std::cout << "  make-index generates labels for 1..R by internal sieve path\n";
    std::cout << "  query uses lookup only\n";
    std::cout << "  no external isprime, no miller-rabin, no omega at query time\n";
}

int main(int argc, char** argv) {
    if (argc < 2) { usage(); return 1; }
    std::string cmd = argv[1];
    if (cmd == "make-index") {
        if (argc < 4) { usage(); return 1; }
        uint64_t L = 1, R = 0;
        if (!parse_u64(argv[2], R) || R < 1) { std::cerr << "error: bad upper bound R\n"; return 2; }
        unsigned th = parse_threads(argc, argv, std::thread::hardware_concurrency());
        uint64_t br = parse_block_rows(argc, argv, 0);
        uint64_t fb = parse_flush_bytes(argc, argv, 0);
        return tau_make_index_csv(nullptr, nullptr, argv[3], L, R, th, br, fb);
    }
    if (cmd == "query") {
        if (argc < 4) { usage(); return 1; }
        uint64_t x = 0;
        if (!parse_u64(argv[3], x)) { std::cerr << "error: bad X\n"; return 2; }
        return tau_query_one(argv[2], x);
    }
    if (cmd == "label") {
        if (argc == 3) return tau_query_loop(argv[2]);
        if (argc < 4) { usage(); return 1; }
        uint64_t x = 0;
        if (!parse_u64(argv[3], x)) { std::cerr << "error: bad X\n"; return 2; }
        return tau_query_one(argv[2], x);
    }
    if (cmd == "twin-top10") {
        if (argc < 3) { usage(); return 1; }
        return tau_twin_top10(argv[2]);
    }
    usage();
    return 1;
}
