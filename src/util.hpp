#pragma once

#include <bit>
#include <cassert>
#include <cstdint>

namespace azalea {

inline int lsb(const std::uint64_t b) {
    assert(b != 0);
    return std::countr_zero(b);
}

inline int pop_lsb(std::uint64_t& b) {
    int sq = lsb(b);
    b &= b - 1; // clear the lowest set bit
    return sq;
}

inline bool more_than_one(const std::uint64_t b) {
    return (b & (b - 1)) != 0;
}

} // namespace azalea
