#ifndef TAU_BINARY_HPP
#define TAU_BINARY_HPP

#include <cstdint>
#include <cstdio>
#include <string>
#include <vector>

static const uint32_t TAU_BIN_VERSION = 2;
static const uint32_t TAU_BIN_MAGIC0 = 0x53554154u; /* TAUS */
static const uint32_t TAU_BIN_MAGIC1 = 0x324e5959u; /* YYN2 */

enum TauClass : uint8_t {
    TAU_UNKNOWN = 0,
    TAU_PRIME = 1,
    TAU_SEMIPRIME = 2,
    TAU_ALMOSTPRIME = 3,
    TAU_COMPOSITE = 4
};

#pragma pack(push, 1)
struct TauBinHeader {
    uint32_t magic0;
    uint32_t magic1;
    uint32_t version;
    uint32_t header_size;
    uint32_t row_size;
    uint32_t flags;
    uint64_t range_l;
    uint64_t range_r;
    uint64_t count;
    uint64_t twin_count;
    uint64_t checksum;
};
#pragma pack(pop)

const char* tau_class_name(uint8_t c);
bool tau_read_header(FILE* f, TauBinHeader& h, std::string& err);
bool tau_open_index(const char* path, FILE** fp, TauBinHeader& h, std::string& err);
bool tau_read_label_at(FILE* f, const TauBinHeader& h, uint64_t x, uint8_t& cls, std::string& err);
uint64_t tau_label_offset(const TauBinHeader& h, uint64_t x);

#endif
