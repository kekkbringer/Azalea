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

constexpr std::uint64_t FILES[8] = {
    0x0101010101010101,
    0x0202020202020202,
    0x0404040404040404,
    0x0808080808080808,
    0x1010101010101010,
    0x2020202020202020,
    0x4040404040404040,
    0x8080808080808080,
};

constexpr std::uint64_t ADJACENT_FILES[8] = {
    FILES[1],
    FILES[0] | FILES[2],
    FILES[1] | FILES[3],
    FILES[2] | FILES[4],
    FILES[3] | FILES[5],
    FILES[4] | FILES[6],
    FILES[5] | FILES[7],
    FILES[6],
};

} // namespace azalea
