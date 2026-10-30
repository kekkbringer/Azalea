#pragma once

#include "util.hpp"

#include <array>
#include <cstddef>
#include <cstdint>

namespace azalea {

using Bitboard = std::uint64_t;

namespace attacks {

namespace detail {

struct Offset {
    int file;
    int rank;
};

// Attack table for a piece that jumps by fixed offsets (pawn captures, knight, king).
template <std::size_t N>
constexpr std::array<Bitboard, 64> make_leaper_table(const std::array<Offset, N>& offsets) {
    std::array<Bitboard, 64> table{};
    for (std::size_t sq = 0; sq < 64; ++sq) {
        const int file = static_cast<int>(sq % 8);
        const int rank = static_cast<int>(sq / 8);
        for (const Offset& o : offsets) {
            const int f = file + o.file;
            const int r = rank + o.rank;
            if (f >= 0 && f < 8 && r >= 0 && r < 8) {
                table[sq] |= 1ULL << (8 * r + f);
            }
        }
    }
    return table;
}

inline constexpr auto white_pawn_table =
    make_leaper_table(std::array<Offset, 2>{{{-1, 1}, {1, 1}}});
inline constexpr auto black_pawn_table =
    make_leaper_table(std::array<Offset, 2>{{{-1, -1}, {1, -1}}});
inline constexpr auto knight_table = make_leaper_table(
    std::array<Offset, 8>{{{1, 2}, {2, 1}, {2, -1}, {1, -2}, {-1, -2}, {-2, -1}, {-2, 1}, {-1, 2}}});
inline constexpr auto king_table = make_leaper_table(
    std::array<Offset, 8>{{{1, 0}, {1, 1}, {0, 1}, {-1, 1}, {-1, 0}, {-1, -1}, {0, -1}, {1, -1}}});

} // namespace detail

// contains the ray (bishop or rook like) between two squares
// empty if not possible
[[nodiscard]] Bitboard between(const int sq1, const int sq2);

// contains the line through two squares
// empty if not possible
[[nodiscard]] Bitboard line_through(const int sq1, const int sq2);

// Squares attacked by a pawn of the given color standing on `sq`.
[[nodiscard]] constexpr Bitboard pawn(bool white, int sq) noexcept {
    const auto i = static_cast<std::size_t>(sq);
    return white ? detail::white_pawn_table[i] : detail::black_pawn_table[i];
}

[[nodiscard]] constexpr Bitboard knight(int sq) noexcept {
    return detail::knight_table[static_cast<std::size_t>(sq)];
}

[[nodiscard]] constexpr Bitboard king(int sq) noexcept {
    return detail::king_table[static_cast<std::size_t>(sq)];
}

// Sliding attacks, computed on the fly by walking each ray until the first occupied square.
// The blocking square is included (it may hold an enemy piece that can be captured).
[[nodiscard]] Bitboard bishop(int sq, Bitboard occupied) noexcept;
[[nodiscard]] Bitboard rook(int sq, Bitboard occupied) noexcept;
[[nodiscard]] Bitboard queen(int sq, Bitboard occupied) noexcept;
void init();

// white pawn attacks by shifting
[[nodiscard]] constexpr Bitboard pawn_white(const Bitboard wp) {
    return ((wp & ~FILES[0]) << 7)
         | ((wp & ~FILES[7] << 9));
}

// black pawn attacks by shifting
[[nodiscard]] constexpr Bitboard pawn_black(const Bitboard bp) {
    return ((bp & ~FILES[0]) >> 9)
         | ((bp & ~FILES[7] >> 7));
}

} // namespace attacks

} // namespace azalea
