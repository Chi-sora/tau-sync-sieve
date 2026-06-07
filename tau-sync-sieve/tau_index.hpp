#ifndef TAU_INDEX_HPP
#define TAU_INDEX_HPP

#include <cstdint>
#include <string>

int tau_make_index_csv(const char* csv_path, const char* twin_csv_path, const char* out_path, uint64_t L, uint64_t R, unsigned threads, uint64_t block_rows, uint64_t flush_bytes);

#endif
