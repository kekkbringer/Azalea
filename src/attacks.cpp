#include "attacks.hpp"

namespace azalea::attacks {

namespace {

constexpr std::array<detail::Offset, 4> bishop_directions{{{1, 1}, {-1, 1}, {-1, -1}, {1, -1}}};
constexpr std::array<detail::Offset, 4> rook_directions{{{1, 0}, {0, 1}, {-1, 0}, {0, -1}}};

Bitboard slide(int sq, Bitboard occupied,
               const std::array<detail::Offset, 4>& directions) noexcept {
    Bitboard result = 0;
    for (const detail::Offset& d: directions) {
        int file = sq % 8 + d.file;
        int rank = sq / 8 + d.rank;
        while (file >= 0 && file < 8 && rank >= 0 && rank < 8) {
            const Bitboard bit = 1ULL << (8 * rank + file);
            result |= bit;
            if ((occupied & bit) != 0) {
                break;
            }
            file += d.file;
            rank += d.rank;
        }
    }
    return result;
}

} // namespace

Bitboard bishop(int sq, Bitboard occupied) noexcept {
    return slide(sq, occupied, bishop_directions);
}

Bitboard rook(int sq, Bitboard occupied) noexcept {
    return slide(sq, occupied, rook_directions);
}

Bitboard queen(int sq, Bitboard occupied) noexcept {
    return bishop(sq, occupied) | rook(sq, occupied);
}

} // namespace azalea::attacks
