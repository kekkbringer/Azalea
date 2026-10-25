#include "attacks.hpp"
#include "move.hpp" // square constants

#include <catch2/catch_test_macros.hpp>

#include <bit>
#include <initializer_list>

using azalea::Bitboard;
using namespace azalea; // square constants
namespace atk = azalea::attacks;

namespace {

Bitboard squares(std::initializer_list<int> list) {
    Bitboard bb = 0;
    for (const int sq : list) {
        bb |= 1ULL << sq;
    }
    return bb;
}

bool contains(Bitboard bb, int sq) { return (bb & (1ULL << sq)) != 0; }

constexpr Bitboard FILE_H = 0x8080808080808080ULL;
constexpr Bitboard RANK_1 = 0x00000000000000FFULL;

} // namespace

TEST_CASE("Leaper tables are available at compile time", "[attacks]") {
    STATIC_REQUIRE(atk::knight(0) == ((1ULL << 10) | (1ULL << 17))); // a1: c2, b3
    STATIC_REQUIRE(atk::king(63) != 0);
}

TEST_CASE("Pawn attacks", "[attacks]") {
    CHECK(atk::pawn(true, E2) == squares({D3, F3}));
    CHECK(atk::pawn(true, A2) == squares({B3}));
    CHECK(atk::pawn(true, H2) == squares({G3}));
    CHECK(atk::pawn(false, E7) == squares({D6, F6}));
    CHECK(atk::pawn(false, A7) == squares({B6}));
    CHECK(atk::pawn(false, H7) == squares({G6}));

    // Edge ranks have no squares to attack, but the tables must still be defined there.
    CHECK(atk::pawn(true, E8) == 0);
    CHECK(atk::pawn(false, E1) == 0);
}

TEST_CASE("Knight attacks", "[attacks]") {
    CHECK(atk::knight(G1) == squares({E2, F3, H3}));
    CHECK(atk::knight(D4) == squares({C2, E2, B3, F3, B5, F5, C6, E6}));

    CHECK(std::popcount(atk::knight(A1)) == 2);
    CHECK(std::popcount(atk::knight(B1)) == 3);
    CHECK(std::popcount(atk::knight(B2)) == 4);
    CHECK(std::popcount(atk::knight(C3)) == 8);
    CHECK(std::popcount(atk::knight(H8)) == 2);
}

TEST_CASE("King attacks", "[attacks]") {
    CHECK(atk::king(A1) == squares({A2, B1, B2}));
    CHECK(atk::king(E1) == squares({D1, F1, D2, E2, F2}));
    CHECK(std::popcount(atk::king(E4)) == 8);
    CHECK(std::popcount(atk::king(H8)) == 3);
}

TEST_CASE("Leaper attacks are symmetric", "[attacks]") {
    // If a attacks b, then b attacks a (for pawns: with the other color). This catches
    // attacks that wrap around the board edge, because the wrapped-to square doesn't attack back.
    for (int a = 0; a < 64; ++a) {
        for (int b = 0; b < 64; ++b) {
            INFO("a = " << a << ", b = " << b);
            REQUIRE(contains(atk::knight(a), b) == contains(atk::knight(b), a));
            REQUIRE(contains(atk::king(a), b) == contains(atk::king(b), a));
            REQUIRE(contains(atk::pawn(true, a), b) == contains(atk::pawn(false, b), a));
        }
    }
}

TEST_CASE("Sliding attacks on an empty board", "[attacks]") {
    for (int sq = 0; sq < 64; ++sq) {
        INFO("square = " << sq);
        CHECK(std::popcount(atk::rook(sq, 0)) == 14);
    }
    CHECK(std::popcount(atk::bishop(A1, 0)) == 7);
    CHECK(std::popcount(atk::bishop(D4, 0)) == 13);
    CHECK(std::popcount(atk::queen(D4, 0)) == 27);

    // No wrap-around: a rook on h1 only sees the first rank and the h-file.
    CHECK(atk::rook(H1, 0) == ((RANK_1 | FILE_H) & ~squares({H1})));
}

TEST_CASE("Sliding attacks stop at the first blocker, including it", "[attacks]") {
    CHECK(atk::rook(A1, squares({A3, C1})) == squares({A2, A3, B1, C1}));
    CHECK(atk::bishop(D4, squares({F6, B2})) ==
          squares({E5, F6, C5, B6, A7, C3, B2, E3, F2, G1}));

    // Pieces behind the first blocker don't matter.
    CHECK(atk::rook(A1, squares({A3, A5, C1, E1})) == atk::rook(A1, squares({A3, C1})));

    // The slider's own square in the occupancy doesn't block anything.
    CHECK(atk::rook(D4, squares({D4})) == atk::rook(D4, 0));
    CHECK(atk::bishop(D4, squares({D4})) == atk::bishop(D4, 0));
}

TEST_CASE("Queen attacks are rook plus bishop attacks", "[attacks]") {
    const Bitboard occupancies[] = {0, 0x0000'FFFF'0000'FFFFULL, 0x00FF'0000'0000'FF00ULL,
                                    0x1234'5678'9ABC'DEF0ULL};
    for (const Bitboard occ : occupancies) {
        for (int sq = 0; sq < 64; ++sq) {
            INFO("square = " << sq << ", occupancy = " << occ);
            REQUIRE(atk::queen(sq, occ) == (atk::rook(sq, occ) | atk::bishop(sq, occ)));
        }
    }
}
