#include "attacks.hpp"
#include "zobrist.hpp" // for rng

#include <bit>
#include <vector>
#include <span>
#include <algorithm>

namespace azalea::attacks {

namespace {

constexpr std::array<detail::Offset, 4> bishop_directions{{{1, 1}, {-1, 1}, {-1, -1}, {1, -1}}};
constexpr std::array<detail::Offset, 4> rook_directions{{{1, 0}, {0, 1}, {-1, 0}, {0, -1}}};

Bitboard slide(int sq, Bitboard occupied,
               const std::array<detail::Offset, 4>& directions) noexcept {
    Bitboard result = 0ULL;
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

Bitboard gen_mask(int sq, const std::array<detail::Offset, 4>& directions) noexcept {
    Bitboard mask = 0ULL;
    for (const detail::Offset& d: directions) {
        int file = sq % 8 + d.file;
        int rank = sq / 8 + d.rank;
        while (file + d.file >= 0 && file + d.file < 8
            && rank + d.rank >= 0 && rank + d.rank < 8) {
            const Bitboard bit = 1ULL << (8 * rank + file);
            mask |= bit;
            file += d.file;
            rank += d.rank;
        }
    }
    return mask;
}

// plain magic bitboards
constexpr std::size_t ROOK_BITS   = 12;
constexpr std::size_t BISHOP_BITS =  9;

Bitboard rook_mask[64];
Bitboard bishop_mask[64];
Bitboard rook_magics[64];
Bitboard bishop_magics[64];

Bitboard rook_table[64][4096];
Bitboard bishop_table[64][512];


// magic finder
std::uint64_t seed = 0xA5AB9164BA3CFE11;
Bitboard find_magic(const int sq, const int bits, const Bitboard mask,
                    const std::array<detail::Offset, 4>& directions,
                    std::span<Bitboard> table) {
    // generate all possible (and relevant) occupancies
    std::size_t count = 0;
    Bitboard subset = 0ULL;
    std::vector<Bitboard> occs(1ULL << bits);
    std::vector<Bitboard> ref_atks(1ULL << bits);
    do {
        occs[count] = subset;
        ref_atks[count] = slide(sq, subset, directions);
        ++count;
        subset = (subset - mask) & mask; // carry rippler
    } while (subset != 0);

    // brute force sparse randoms
    while (true) {
        const auto magic = splitmix64(seed) & splitmix64(seed) & splitmix64(seed);

        // does the magic even shift any relevant bits into the upper ones?
        if (std::popcount((mask * magic) & 0xFF00000000000000) < 7) continue;

        // clear table for every new attempt
        std::ranges::fill(table.first(1ULL << bits), Bitboard{0});
        bool fine = true;
        std::uint64_t trial_index = 0;
        for (std::size_t i=0; i<count; ++i) {
            trial_index = (occs[i] * magic) >> (64 - bits);
            // unused -> fine
            if (table[trial_index] == 0ULL) {
                table[trial_index] = ref_atks[i];
            // destructive collision -> not fine
            } else if (table[trial_index] != ref_atks[i]) {
                fine = false;
                break;
            }
            // else: constructive collision -> also fine, nothing to do
        }
        if (fine) return magic;
    }
}

// ray between squares
Bitboard between_table[64][64];

void fill_between() {
    for (std::size_t i=0; i<64; ++i) {
        const auto i_rook   = rook(static_cast<int>(i), 0);
        const auto i_bishop = bishop(static_cast<int>(i), 0);
        const Bitboard i_mask = (1ULL << i);
        for (std::size_t j=0; j<64; ++j) {
            between_table[i][j] = 0ULL;
            const Bitboard j_mask = (1ULL << j);
            if (j_mask & i_rook)
                between_table[i][j] |= rook(static_cast<int>(j), i_mask)
                                     & rook(static_cast<int>(i), j_mask);
            if (j_mask & i_bishop)
                between_table[i][j] |= bishop(static_cast<int>(j), i_mask)
                                     & bishop(static_cast<int>(i), j_mask);
        }
    }
}

// line through squares
Bitboard line_table[64][64];

void fill_line() {
    for (std::size_t i=0; i<64; ++i) {
        const auto i_rook   = rook(static_cast<int>(i), 0);
        const auto i_bishop = bishop(static_cast<int>(i), 0);
        const Bitboard i_mask = (1ULL << i);
        for (std::size_t j=0; j<64; ++j) {
            line_table[i][j] = 0ULL;
            const Bitboard j_mask = (1ULL << j);
            if (j_mask & i_rook)
                line_table[i][j] |= (rook(static_cast<int>(j), 0) | j_mask)
                                  & (i_rook | i_mask);
            if (j_mask & i_bishop)
                line_table[i][j] |= (bishop(static_cast<int>(j), 0) | j_mask)
                                  & (i_bishop | i_mask);
        }
    }
}

} // namespace


[[nodiscard]] Bitboard between(const int sq1, const int sq2) {
    return between_table[static_cast<std::size_t>(sq1)][static_cast<std::size_t>(sq2)];
}

[[nodiscard]] Bitboard line_through(const int sq1, const int sq2) {
    return line_table[static_cast<std::size_t>(sq1)][static_cast<std::size_t>(sq2)];
}

// init magics
void init() {
    // init masks and magics
    for (int sq=0; sq<64; ++sq) {
        rook_mask[sq] = gen_mask(sq, rook_directions);
        rook_magics[sq] = find_magic(sq, ROOK_BITS, rook_mask[sq], rook_directions, rook_table[sq]);

        bishop_mask[sq] = gen_mask(sq, bishop_directions);
        bishop_magics[sq] = find_magic(sq, BISHOP_BITS, bishop_mask[sq], bishop_directions, bishop_table[sq]);
    }

    // between and line table
    fill_between();
    fill_line();
}


Bitboard bishop(int sq, Bitboard occupied) noexcept {
    const auto index = ((occupied & bishop_mask[sq]) * bishop_magics[sq]) >> (64 - BISHOP_BITS);
    return bishop_table[sq][index];
}

Bitboard rook(int sq, Bitboard occupied) noexcept {
    const auto index = ((occupied & rook_mask[sq]) * rook_magics[sq]) >> (64 - ROOK_BITS);
    return rook_table[sq][index];
}

Bitboard queen(int sq, Bitboard occupied) noexcept {
    return bishop(sq, occupied) | rook(sq, occupied);
}

} // namespace azalea::attacks
