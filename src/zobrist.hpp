#pragma once

#include <cstdint>
#include <array>

namespace azalea {

constexpr uint64_t splitmix64(std::uint64_t& state) {
    std::uint64_t res = (state += 0x9E3779B97F4A7C15ULL);
    res = (res ^ (res >> 30)) * 0xBF58476D1CE4E5B9ULL;
    res = (res ^ (res >> 27)) * 0x94D049BB133111EBULL;
    return res ^ (res >> 31);
}

struct ZobristKeys {
    std::array<std::array<std::uint64_t, 64>, 12> piece_square{};
    std::array<std::uint64_t, 16> castling{};
    std::array<std::uint64_t,  8> en_passant{};
    std::uint64_t side_to_move{};
};

constexpr ZobristKeys gen_keys(std::uint64_t seed) {
    ZobristKeys keys{};
    // piece + square combinations
    for (auto& piece_keys: keys.piece_square)
        for (auto& key: piece_keys)
            key = splitmix64(seed);
    // castling
    for (auto& key: keys.castling)
        key = splitmix64(seed);
    // en passant
    for (auto& key: keys.en_passant)
        key = splitmix64(seed);
    // side to move
    keys.side_to_move = splitmix64(seed);

    return keys;
}

inline constexpr ZobristKeys z_keys = gen_keys(0x2545F4914F6CDD1DULL);

} // namespace azalea
