#include "tau_binary.hpp"

const char* tau_class_name(uint8_t c) {
    switch (c) {
        case TAU_PRIME: return "prime";
        case TAU_SEMIPRIME: return "semiprime";
        case TAU_ALMOSTPRIME: return "almostprime";
        case TAU_COMPOSITE: return "composite";
        default: return "unknown";
    }
}

uint64_t tau_label_offset(const TauBinHeader& h, uint64_t x) {
    return (uint64_t)h.header_size + (x - h.range_l) * (uint64_t)h.row_size;
}

bool tau_read_header(FILE* f, TauBinHeader& h, std::string& err) {
    if (std::fread(&h, sizeof(h), 1, f) != 1) { err = "header read failed"; return false; }
    if (h.magic0 != TAU_BIN_MAGIC0 || h.magic1 != TAU_BIN_MAGIC1) { err = "bad magic"; return false; }
    if (h.version != TAU_BIN_VERSION) { err = "bad version"; return false; }
    if (h.header_size != sizeof(TauBinHeader)) { err = "bad header size"; return false; }
    if (h.row_size != 1) { err = "bad row size"; return false; }
    if (h.range_l != 1) { err = "bad range_l"; return false; }
    if (h.range_r < h.range_l) { err = "bad range"; return false; }
    if (h.count != h.range_r - h.range_l + 1) { err = "bad count"; return false; }
    return true;
}

bool tau_open_index(const char* path, FILE** fp, TauBinHeader& h, std::string& err) {
    FILE* f = std::fopen(path, "rb");
    if (!f) { err = "cannot open index"; return false; }
    if (!tau_read_header(f, h, err)) { std::fclose(f); return false; }
    *fp = f;
    return true;
}

bool tau_read_label_at(FILE* f, const TauBinHeader& h, uint64_t x, uint8_t& cls, std::string& err) {
    if (x < h.range_l || x > h.range_r) { cls = TAU_UNKNOWN; return true; }
    uint64_t off = tau_label_offset(h, x);
#if defined(_WIN32)
    if (_fseeki64(f, (long long)off, SEEK_SET) != 0) { err = "seek failed"; return false; }
#else
    if (fseeko(f, (off_t)off, SEEK_SET) != 0) { err = "seek failed"; return false; }
#endif
    int c = std::fgetc(f);
    if (c == EOF) { err = "label read failed"; return false; }
    cls = (uint8_t)c;
    return true;
}
