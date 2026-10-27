#include "board.hpp"

#include <catch2/catch_test_macros.hpp>

using azalea::Board;
using azalea::Move;
using namespace azalea; // square and flag constants

// is_attacked(sq, by_white): is `sq` attacked by the given side's pieces?

TEST_CASE("is_attacked: pawns attack diagonally forward only", "[board][legality]") {
    const Board board{"4k3/8/8/4p3/4P3/8/8/4K3 w - - 0 1"};
    CHECK(board.is_attacked(D5, true));
    CHECK(board.is_attacked(F5, true));
    CHECK_FALSE(board.is_attacked(E5, true));

    CHECK(board.is_attacked(D4, false));
    CHECK(board.is_attacked(F4, false));
    CHECK_FALSE(board.is_attacked(E4, false));
    CHECK_FALSE(board.is_attacked(D3, false));
}

TEST_CASE("is_attacked: knights", "[board][legality]") {
    const Board board{"4k3/8/8/8/3N4/8/8/4K3 w - - 0 1"};
    for (const int sq : {C6, E6, F5, F3, B3, B5, C2}) {
        INFO("square = " << sq);
        CHECK(board.is_attacked(sq, true));
    }
    CHECK_FALSE(board.is_attacked(D5, true));
    CHECK_FALSE(board.is_attacked(D6, true));
}

TEST_CASE("is_attacked: sliders stop at the first blocker", "[board][legality]") {
    SECTION("bishop") {
        const Board board{"4k3/8/8/8/3B4/8/1P6/4K3 w - - 0 1"};
        CHECK(board.is_attacked(A7, true));
        CHECK(board.is_attacked(H8, true));
        CHECK(board.is_attacked(G1, true));
        CHECK_FALSE(board.is_attacked(A1, true)); // behind the pawn on b2
    }
    SECTION("rook") {
        const Board board{"4k3/8/8/8/R2p4/8/8/4K3 w - - 0 1"};
        CHECK(board.is_attacked(A8, true));
        CHECK(board.is_attacked(A1, true));
        CHECK(board.is_attacked(D4, true)); // the blocker itself can be captured
        CHECK_FALSE(board.is_attacked(E4, true));
        CHECK_FALSE(board.is_attacked(H4, true));
    }
    SECTION("own pieces block too") {
        const Board board{"r3k3/8/8/8/8/8/P7/R3K3 b - - 0 1"};
        CHECK_FALSE(board.is_attacked(A8, true));
    }
}

TEST_CASE("is_attacked: queens attack like rooks and bishops", "[board][legality]") {
    const Board board{"4k3/8/8/8/3Q4/8/8/4K3 w - - 0 1"};
    CHECK(board.is_attacked(H8, true));
    CHECK(board.is_attacked(D8, true));
    CHECK(board.is_attacked(A4, true));
    CHECK(board.is_attacked(A1, true));
    CHECK_FALSE(board.is_attacked(E6, true));
}

TEST_CASE("is_attacked: kings", "[board][legality]") {
    const Board board{"4k3/8/8/8/8/8/8/4K3 w - - 0 1"};
    CHECK(board.is_attacked(D7, false));
    CHECK(board.is_attacked(F8, false));
    CHECK_FALSE(board.is_attacked(E6, false));
    CHECK(board.is_attacked(D2, true));
}

TEST_CASE("is_attacked: depends on the attacker, not the side to move", "[board][legality]") {
    const Board white_to_move{"4k3/8/8/8/8/8/8/R3K3 w - - 0 1"};
    const Board black_to_move{"4k3/8/8/8/8/8/8/R3K3 b - - 0 1"};
    for (const Board* board : {&white_to_move, &black_to_move}) {
        INFO("white to move = " << board->white_to_move);
        CHECK(board->is_attacked(A8, true));
        CHECK_FALSE(board->is_attacked(A8, false));
        CHECK(board->is_attacked(D8, false)); // black king
        CHECK_FALSE(board->is_attacked(D8, true));
    }
}

TEST_CASE("is_legal: the king may not move into check", "[board][legality]") {
    Board board{"4k3/8/8/8/8/8/8/3RK3 b - - 0 1"};
    CHECK_FALSE(board.is_legal(Move{0, E8, D8}));
    CHECK_FALSE(board.is_legal(Move{0, E8, D7}));
    CHECK(board.is_legal(Move{0, E8, F8}));
    CHECK(board.is_legal(Move{0, E8, E7}));
}

TEST_CASE("is_legal: a check must be resolved", "[board][legality]") {
    SECTION("black king in check by a rook") {
        Board board{"4k3/8/8/8/8/8/8/4RK2 b - - 0 1"};
        CHECK_FALSE(board.is_legal(Move{0, E8, E7})); // still on the e-file
        CHECK(board.is_legal(Move{0, E8, D8}));
        CHECK(board.is_legal(Move{0, E8, D7}));
    }
    SECTION("white king in check by a bishop") {
        Board board{"4k3/8/8/8/1b6/8/8/4K3 w - - 0 1"};
        CHECK_FALSE(board.is_legal(Move{0, E1, D2})); // still on the diagonal
        CHECK(board.is_legal(Move{0, E1, E2}));
        CHECK(board.is_legal(Move{0, E1, D1}));
        CHECK(board.is_legal(Move{0, E1, F2}));
    }
}

TEST_CASE("is_legal: pinned pieces", "[board][legality]") {
    SECTION("pinned bishop cannot leave the pin line") {
        Board board{"4k3/4b3/8/8/8/8/8/4RK2 b - - 0 1"};
        CHECK_FALSE(board.is_legal(Move{0, E7, D6}));
    }
    SECTION("pinned rook may move along the pin line") {
        Board board{"4k3/4r3/8/8/8/8/8/4RK2 b - - 0 1"};
        CHECK(board.is_legal(Move{0, E7, E5}));
        CHECK(board.is_legal(Move{CAPTURE, E7, E1}));
        CHECK_FALSE(board.is_legal(Move{0, E7, D7}));
    }
    SECTION("en passant that exposes the king along the rank") {
        // Capturing removes both pawns from the 5th rank, so the rook on h5 attacks a5.
        Board board{"8/8/8/K2pP2r/8/8/8/7k w - d6 0 1"};
        CHECK_FALSE(board.is_legal(Move{EN_PASSANT, E5, D6}));
    }
}

TEST_CASE("is_legal: castling", "[board][legality]") {
    SECTION("allowed when the king's path is safe") {
        CHECK(Board{"4k3/8/8/8/8/8/8/4K2R w K - 0 1"}.is_legal(Move{KING_SIDE_CASTLE, E1, G1}));
    }
    SECTION("not out of check") {
        CHECK_FALSE(
            Board{"4k3/8/8/8/8/8/8/r3K2R w K - 0 1"}.is_legal(Move{KING_SIDE_CASTLE, E1, G1}));
    }
    SECTION("not through an attacked square") {
        CHECK_FALSE(
            Board{"4kr2/8/8/8/8/8/8/4K2R w K - 0 1"}.is_legal(Move{KING_SIDE_CASTLE, E1, G1}));
    }
    SECTION("not into check") {
        CHECK_FALSE(
            Board{"4k1r1/8/8/8/8/8/8/4K2R w K - 0 1"}.is_legal(Move{KING_SIDE_CASTLE, E1, G1}));
    }
    SECTION("allowed when only the rook is attacked") {
        CHECK(Board{"4k2r/8/8/8/8/8/8/4K2R w K - 0 1"}.is_legal(Move{KING_SIDE_CASTLE, E1, G1}));
    }
    SECTION("queen side: b1 may be attacked, d1 may not") {
        CHECK(
            Board{"1r2k3/8/8/8/8/8/8/R3K3 w Q - 0 1"}.is_legal(Move{QUEEN_SIDE_CASTLE, E1, C1}));
        CHECK_FALSE(
            Board{"3rk3/8/8/8/8/8/8/R3K3 w Q - 0 1"}.is_legal(Move{QUEEN_SIDE_CASTLE, E1, C1}));
    }
    SECTION("black, through an attacked square") {
        CHECK_FALSE(
            Board{"4k2r/8/8/8/8/8/8/4KR2 b k - 0 1"}.is_legal(Move{KING_SIDE_CASTLE, E8, G8}));
    }
}
