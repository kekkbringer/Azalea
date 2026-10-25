#include "move.hpp"

#include <catch2/catch_test_macros.hpp>

#include <array>

using azalea::Move;

namespace {

// Expected result of every flag predicate for one flag value.
struct FlagCase {
    int flag;
    bool quiet;
    bool double_pawn_push;
    bool king_side_castle;
    bool queen_side_castle;
    bool capture;
    bool en_passant;
    bool promotion;
    bool knight_promotion;
    bool bishop_promotion;
    bool rook_promotion;
    bool queen_promotion;
};

// Flags 6 and 7 are unused and therefore not listed.
// clang-format off
constexpr std::array<FlagCase, 14> flag_cases{{
    // flag  quiet  dpp    ksc    qsc    capt   ep     promo  N      B      R      Q
    {  0,    true,  false, false, false, false, false, false, false, false, false, false },
    {  1,    true,  true,  false, false, false, false, false, false, false, false, false },
    {  2,    true,  false, true,  false, false, false, false, false, false, false, false },
    {  3,    true,  false, false, true,  false, false, false, false, false, false, false },
    {  4,    false, false, false, false, true,  false, false, false, false, false, false },
    {  5,    false, false, false, false, true,  true,  false, false, false, false, false },
    {  8,    false, false, false, false, false, false, true,  true,  false, false, false },
    {  9,    false, false, false, false, false, false, true,  false, true,  false, false },
    { 10,    false, false, false, false, false, false, true,  false, false, true,  false },
    { 11,    false, false, false, false, false, false, true,  false, false, false, true  },
    { 12,    false, false, false, false, true,  false, true,  true,  false, false, false },
    { 13,    false, false, false, false, true,  false, true,  false, true,  false, false },
    { 14,    false, false, false, false, true,  false, true,  false, false, true,  false },
    { 15,    false, false, false, false, true,  false, true,  false, false, false, true  },
}};
// clang-format on

} // namespace

TEST_CASE("A default-constructed move is all zero", "[move]") {
    constexpr Move move;
    STATIC_REQUIRE(move.info == 0);
    STATIC_REQUIRE(move.flag() == 0);
    STATIC_REQUIRE(move.from() == 0);
    STATIC_REQUIRE(move.to() == 0);
}

TEST_CASE("Move accessors are usable at compile time", "[move]") {
    constexpr Move move{azalea::QUEEN_PROMOTION | azalea::CAPTURE, 52, 63};
    STATIC_REQUIRE(move.flag() == 15);
    STATIC_REQUIRE(move.from() == 52);
    STATIC_REQUIRE(move.to() == 63);
    STATIC_REQUIRE(move.is_queen_promotion());
}

TEST_CASE("Move stores flag, from and to in the documented bit layout", "[move]") {
    // flag | to | from  ->  bits 15-12 | 11-6 | 5-0
    CHECK(Move{0, 1, 0}.info == 0b0000'000000'000001);
    CHECK(Move{0, 0, 1}.info == 0b0000'000001'000000);
    CHECK(Move{1, 0, 0}.info == 0b0001'000000'000000);
    CHECK(Move{15, 63, 63}.info == 0xFFFF);
}

TEST_CASE("Move round-trips every flag, from and to combination", "[move]") {
    // Fields must not bleed into each other, so check all 16 * 64 * 64 combinations.
    int mismatches = 0;
    for (int flag = 0; flag < 16; ++flag) {
        for (int from = 0; from < 64; ++from) {
            for (int to = 0; to < 64; ++to) {
                const Move move{flag, from, to};
                if (move.flag() != flag || move.from() != from || move.to() != to) {
                    if (mismatches == 0) {
                        UNSCOPED_INFO("first mismatch: flag=" << flag << " from=" << from
                                                              << " to=" << to);
                    }
                    ++mismatches;
                }
            }
        }
    }
    CHECK(mismatches == 0);
}

TEST_CASE("Two-argument constructor creates a normal move", "[move]") {
    const Move move{12, 28};
    CHECK(move.flag() == 0);
    CHECK(move.from() == 12);
    CHECK(move.to() == 28);
    CHECK(move == Move{0, 12, 28});
}

TEST_CASE("Flag predicates match the flag table", "[move]") {
    for (const FlagCase& c : flag_cases) {
        INFO("flag = " << c.flag);
        const Move move{c.flag, 12, 28};
        CHECK(move.is_quiet() == c.quiet);
        CHECK(move.is_double_pawn_push() == c.double_pawn_push);
        CHECK(move.is_king_side_castle() == c.king_side_castle);
        CHECK(move.is_queen_side_castle() == c.queen_side_castle);
        CHECK(move.is_capture() == c.capture);
        CHECK(move.is_en_passant() == c.en_passant);
        CHECK(move.is_promotion() == c.promotion);
        CHECK(move.is_knight_promotion() == c.knight_promotion);
        CHECK(move.is_bishop_promotion() == c.bishop_promotion);
        CHECK(move.is_rook_promotion() == c.rook_promotion);
        CHECK(move.is_queen_promotion() == c.queen_promotion);
    }
}

TEST_CASE("Flag predicates do not depend on the squares", "[move]") {
    // Squares with all bits set must not make flag checks fire.
    const Move move{0, 63, 63};
    CHECK(move.is_quiet());
    CHECK_FALSE(move.is_capture());
    CHECK_FALSE(move.is_promotion());
    CHECK_FALSE(move.is_double_pawn_push());
}

TEST_CASE("Moves compare equal only if all fields match", "[move]") {
    CHECK(Move{azalea::CAPTURE, 12, 28} == Move{azalea::CAPTURE, 12, 28});
    CHECK_FALSE(Move{azalea::CAPTURE, 12, 28} == Move{0, 12, 28});
    CHECK_FALSE(Move{0, 12, 28} == Move{0, 28, 12});
    CHECK_FALSE(Move{0, 12, 28} == Move{0, 12, 29});
}
