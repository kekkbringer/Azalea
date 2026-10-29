#pragma once

#include <cstdint>
#include <string>
#include <string_view>

namespace azalea {
/**
 * Helper for individual squares of the board
 *   |  A  B  C  D  E  F  G  H
 * --+-------------------------
 * 8 | 56 57 58 59 60 61 62 63
 * 7 | 48 49 50 51 52 53 54 55
 * 6 | 40 41 42 43 44 45 46 47
 * 5 | 32 33 34 35 36 37 38 39
 * 4 | 24 25 26 27 28 29 30 31
 * 3 | 16 17 18 19 20 21 22 23
 * 2 |  8  9 10 11 12 13 14 15
 * 1 |  0  1  2  3  4  5  6  7
 */
// clang-format off
constexpr int A1 =  0; constexpr int B1 =  1; constexpr int C1 =  2; constexpr int D1 =  3;
constexpr int E1 =  4; constexpr int F1 =  5; constexpr int G1 =  6; constexpr int H1 =  7;
constexpr int A2 =  8; constexpr int B2 =  9; constexpr int C2 = 10; constexpr int D2 = 11;
constexpr int E2 = 12; constexpr int F2 = 13; constexpr int G2 = 14; constexpr int H2 = 15;
constexpr int A3 = 16; constexpr int B3 = 17; constexpr int C3 = 18; constexpr int D3 = 19;
constexpr int E3 = 20; constexpr int F3 = 21; constexpr int G3 = 22; constexpr int H3 = 23;
constexpr int A4 = 24; constexpr int B4 = 25; constexpr int C4 = 26; constexpr int D4 = 27;
constexpr int E4 = 28; constexpr int F4 = 29; constexpr int G4 = 30; constexpr int H4 = 31;
constexpr int A5 = 32; constexpr int B5 = 33; constexpr int C5 = 34; constexpr int D5 = 35;
constexpr int E5 = 36; constexpr int F5 = 37; constexpr int G5 = 38; constexpr int H5 = 39;
constexpr int A6 = 40; constexpr int B6 = 41; constexpr int C6 = 42; constexpr int D6 = 43;
constexpr int E6 = 44; constexpr int F6 = 45; constexpr int G6 = 46; constexpr int H6 = 47;
constexpr int A7 = 48; constexpr int B7 = 49; constexpr int C7 = 50; constexpr int D7 = 51;
constexpr int E7 = 52; constexpr int F7 = 53; constexpr int G7 = 54; constexpr int H7 = 55;
constexpr int A8 = 56; constexpr int B8 = 57; constexpr int C8 = 58; constexpr int D8 = 59;
constexpr int E8 = 60; constexpr int F8 = 61; constexpr int G8 = 62; constexpr int H8 = 63;

constexpr int alg_to_int(const std::string_view& alg) {
    return (alg[0]-'a') + 8 * (alg[1]-'1');
}

constexpr std::string_view int_to_alg(const int sq) {
    constexpr std::string_view algs[64] = {"a1", "b1", "c1", "d1", "e1", "f1", "g1", "h1",
                                           "a2", "b2", "c2", "d2", "e2", "f2", "g2", "h2",
                                           "a3", "b3", "c3", "d3", "e3", "f3", "g3", "h3",
                                           "a4", "b4", "c4", "d4", "e4", "f4", "g4", "h4",
                                           "a5", "b5", "c5", "d5", "e5", "f5", "g5", "h5",
                                           "a6", "b6", "c6", "d6", "e6", "f6", "g6", "h6",
                                           "a7", "b7", "c7", "d7", "e7", "f7", "g7", "h7",
                                           "a8", "b8", "c8", "d8", "e8", "f8", "g8", "h8"};
    return algs[sq];
}
// clang-format on


/**
 * Flags to indicate special moves:
 *   0  |  0000  |  normal move
 *   1  |  0001  |  double pawn push
 *   2  |  0010  |  king-side castle
 *   3  |  0011  |  queen-side castle
 *   4  |  0100  |  capture
 *   5  |  0101  |  en passant capture
 *   6  |  0110  |  ---
 *   7  |  0111  |  ---
 *   8  |  1000  |  promotion to knight
 *   9  |  1001  |  promotion to bishop
 *  10  |  1010  |  promotion to rook
 *  11  |  1011  |  promotion to queen
 *  12  |  1100  |  capture promotion to knight
 *  13  |  1101  |  capture promotion to bishop
 *  14  |  1110  |  capture promotion to rook
 *  15  |  1111  |  capture promotion to queen
 */
constexpr std::uint16_t DOUBLE_PAWN_PUSH  =  1;
constexpr std::uint16_t KING_SIDE_CASTLE  =  2;
constexpr std::uint16_t QUEEN_SIDE_CASTLE =  3;
constexpr std::uint16_t CAPTURE           =  4;
constexpr std::uint16_t EN_PASSANT        =  5;
constexpr std::uint16_t PROMOTION         =  8;
constexpr std::uint16_t KNIGHT_PROMOTION  =  8;
constexpr std::uint16_t BISHOP_PROMOTION  =  9;
constexpr std::uint16_t ROOK_PROMOTION    = 10;
constexpr std::uint16_t QUEEN_PROMOTION   = 11;

/**
 * Class for a chess move encoded as a 16 bit integer. The first 4 bit are flag
 * bits, the next 6 bits indicate the "to" square and the last 6 bits the "from"
 * square of the move.
 *
 *           flag  |      to     |     from
 * info    1 2 3 4 | 5 6 7 8 9 A | B C D E F G
 */
struct Move {
    std::uint16_t info{};

    constexpr Move() = default;
    constexpr Move(const int from, const int to) {
        info = static_cast<std::uint16_t>((to & 63) << 6) | (from & 63);
    }
    constexpr Move(const int flag, const int from, const int to) {
        info = static_cast<std::uint16_t>((flag << 12) | ((to & 63) << 6) | (from & 63));
    }

    [[nodiscard]] constexpr std::uint16_t flag() const noexcept { return info >> 12; }
    [[nodiscard]] constexpr std::uint16_t to() const noexcept { return (info >> 6) & 63; }
    [[nodiscard]] constexpr std::uint16_t from() const noexcept { return info & 63; }

    [[nodiscard]] constexpr bool is_quiet() const noexcept {
        return not (this->is_capture() or this->is_promotion());
    }
    [[nodiscard]] constexpr bool is_double_pawn_push() const noexcept {
        return this->flag() == DOUBLE_PAWN_PUSH;
    }
    [[nodiscard]] constexpr bool is_king_side_castle() const noexcept {
        return this->flag() == KING_SIDE_CASTLE;
    }
    [[nodiscard]] constexpr bool is_queen_side_castle() const noexcept {
        return this->flag() == QUEEN_SIDE_CASTLE;
    }
    [[nodiscard]] constexpr bool is_castling() const noexcept {
        return is_king_side_castle() or is_queen_side_castle();
    }
    [[nodiscard]] constexpr bool is_capture() const noexcept {
        return (this->flag() & CAPTURE) != 0;
    }
    [[nodiscard]] constexpr bool is_en_passant() const noexcept {
        return this->flag() == EN_PASSANT;
    }
    [[nodiscard]] constexpr bool is_promotion() const noexcept {
        return (this->flag() & PROMOTION) != 0;
    }
    [[nodiscard]] constexpr bool is_knight_promotion() const noexcept {
        return this->is_promotion() and (this->flag() & 3) == (KNIGHT_PROMOTION & 3);
    }
    [[nodiscard]] constexpr bool is_bishop_promotion() const noexcept {
        return this->is_promotion() and (this->flag() & 3) == (BISHOP_PROMOTION & 3);
    }
    [[nodiscard]] constexpr bool is_rook_promotion()  const noexcept {
        return this->is_promotion() and (this->flag() & 3) == (ROOK_PROMOTION & 3);
    }
    [[nodiscard]] constexpr bool is_queen_promotion() const noexcept {
        return this->is_promotion() and (this->flag() & 3) == (QUEEN_PROMOTION & 3);
    }

    [[nodiscard]] constexpr bool operator==(const Move&) const noexcept = default;
    [[nodiscard]] constexpr bool operator!=(const Move&) const noexcept = default;
};

inline std::string to_uci(const Move& move) {
    std::string name;

    name = std::string(int_to_alg(move.from()))
         + std::string(int_to_alg(move.to()));

    if (move.is_knight_promotion()) name += "n";
    if (move.is_bishop_promotion()) name += "b";
    if (move.is_rook_promotion()) name += "r";
    if (move.is_queen_promotion()) name += "q";

    return name;
}

} // namespace azalea
