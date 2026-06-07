#include "tau_query.hpp"
#include "tau_binary.hpp"
#include <cstdio>
#include <iostream>
#include <string>
#include <cstdint>

static bool parse_u64_ascii(const std::string& s, uint64_t& out) {
    if (s.empty()) return false;
    uint64_t v = 0;
    for (char ch : s) {
        if (ch < '0' || ch > '9') return false;
        uint64_t d = (uint64_t)(ch - '0');
        if (v > (UINT64_MAX - d) / 10) return false;
        v = v * 10 + d;
    }
    out = v;
    return true;
}

static void print_label(FILE* f, const TauBinHeader& h, uint64_t x) {
    uint8_t cls = TAU_UNKNOWN;
    std::string err;
    if (!tau_read_label_at(f, h, x, cls, err)) {
        std::cerr << "error: " << err << "\n";
        return;
    }
    if (x < h.range_l || x > h.range_r) {
        std::cout << x << " unknown out_of_index_range\n";
        return;
    }
    std::cout << x << " " << tau_class_name(cls) << " residue210=" << (x % 210ull) << "\n";
}

int tau_query_one(const char* index_path, uint64_t x) {
    FILE* f = nullptr;
    TauBinHeader h{};
    std::string err;
    if (!tau_open_index(index_path, &f, h, err)) { std::cerr << "error: " << err << "\n"; return 2; }
    print_label(f, h, x);
    std::fclose(f);
    return 0;
}

int tau_query_loop(const char* index_path) {
    FILE* f = nullptr;
    TauBinHeader h{};
    std::string err;
    if (!tau_open_index(index_path, &f, h, err)) { std::cerr << "error: " << err << "\n"; return 2; }
    std::cout << "input integer. q to quit.\n";
    std::string line;
    while (true) {
        std::cout << "x> ";
        if (!std::getline(std::cin, line)) break;
        if (line == "q" || line == "quit" || line == "exit") break;
        uint64_t x = 0;
        if (!parse_u64_ascii(line, x)) {
            std::cout << "bad input\n";
            continue;
        }
        print_label(f, h, x);
    }
    std::fclose(f);
    return 0;
}
