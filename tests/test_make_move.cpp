#include "board.hpp"

#include <catch2/catch_test_macros.hpp>

#include <cstdint>
#include <string>

using azalea::Board;
using azalea::Move;
using namespace azalea; // square and flag constants

namespace {

// Plays `move` on the position `fen` and checks the resulting FEN and bitboard consistency.
// Expected FENs were cross-checked with python-chess (en passant square written after every
// double pawn push, as Board::make_move does).
void check_move(const std::string& fen, Move move, const std::string& expected) {
    INFO("start: " << fen);
    INFO("move:  " << to_uci(move));

    Board board{fen};
    board.make_move(move);

    CHECK(board.to_fen() == expected);

    // No square belongs to both colors, and colors and piece types cover the same squares.
    CHECK((board.white_pieces & board.black_pieces) == 0);
    const std::uint64_t all_types = board.pawns | board.knights | board.bishops | board.rooks |
                                    board.queens | board.kings;
    CHECK(all_types == (board.white_pieces | board.black_pieces));
}

constexpr const char* start_fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
constexpr const char* castling_fen_white = "r3k2r/8/8/8/8/8/8/R3K2R w KQkq - 0 1";
constexpr const char* castling_fen_black = "r3k2r/8/8/8/8/8/8/R3K2R b KQkq - 0 1";

} // namespace

TEST_CASE("make_move: quiet moves", "[board][make_move]") {
    SECTION("single pawn push resets the halfmove clock") {
        check_move(start_fen, Move{0, E2, E3},
                   "rnbqkbnr/pppppppp/8/8/8/4P3/PPPP1PPP/RNBQKBNR b KQkq - 0 1");
    }
    SECTION("knight move increments the halfmove clock") {
        check_move(start_fen, Move{0, G1, F3},
                   "rnbqkbnr/pppppppp/8/8/8/5N2/PPPPPPPP/RNBQKB1R b KQkq - 1 1");
    }
}

TEST_CASE("make_move: double pawn pushes set the en passant target only if it can be used",
          "[board][make_move]") {
    // The square is only set when an enemy pawn could capture onto it. Otherwise the
    // position is the same as without it, which matters for repetition detection.
    SECTION("white, enemy pawn on the left") {
        check_move("4k3/8/8/8/3p4/8/4P3/4K3 w - - 0 1", Move{DOUBLE_PAWN_PUSH, E2, E4},
                   "4k3/8/8/8/3pP3/8/8/4K3 b - e3 0 1");
    }
    SECTION("white, enemy pawn on the right") {
        check_move("4k3/8/8/8/5p2/8/4P3/4K3 w - - 0 1", Move{DOUBLE_PAWN_PUSH, E2, E4},
                   "4k3/8/8/8/4Pp2/8/8/4K3 b - e3 0 1");
    }
    SECTION("black, which also increments the fullmove number") {
        check_move("4k3/4p3/8/3P4/8/8/8/4K3 b - - 0 1", Move{DOUBLE_PAWN_PUSH, E7, E5},
                   "4k3/8/8/3Pp3/8/8/8/4K3 w - e6 0 2");
    }
    SECTION("no enemy pawn next to it: from the start position") {
        check_move(start_fen, Move{DOUBLE_PAWN_PUSH, E2, E4},
                   "rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq - 0 1");
        check_move("rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq - 0 1",
                   Move{DOUBLE_PAWN_PUSH, E7, E5},
                   "rnbqkbnr/pppp1ppp/8/4p3/4P3/8/PPPP1PPP/RNBQKBNR w KQkq - 0 2");
    }
    SECTION("only an own pawn next to it") {
        check_move("4k3/8/8/8/3P4/8/4P3/4K3 w - - 0 1", Move{DOUBLE_PAWN_PUSH, E2, E4},
                   "4k3/8/8/8/3PP3/8/8/4K3 b - - 0 1");
    }
    SECTION("an enemy pawn on the same rank but not adjacent") {
        check_move("4k3/8/8/8/2p5/8/4P3/4K3 w - - 0 1", Move{DOUBLE_PAWN_PUSH, E2, E4},
                   "4k3/8/8/8/2p1P3/8/8/4K3 b - - 0 1");
    }
    SECTION("an a-file push doesn't wrap around to the h-file") {
        check_move("4k3/8/8/8/7p/8/P7/4K3 w - - 0 1", Move{DOUBLE_PAWN_PUSH, A2, A4},
                   "4k3/8/8/8/P6p/8/8/4K3 b - - 0 1");
    }
}

TEST_CASE("make_move: captures", "[board][make_move]") {
    SECTION("pawn takes pawn") {
        check_move("rnbqkbnr/ppp1pppp/8/3p4/4P3/8/PPPP1PPP/RNBQKBNR w KQkq d6 0 2",
                   Move{CAPTURE, E4, D5},
                   "rnbqkbnr/ppp1pppp/8/3P4/8/8/PPPP1PPP/RNBQKBNR b KQkq - 0 2");
    }
    SECTION("white knight takes knight") {
        check_move("4k3/8/8/3n4/8/4N3/8/4K3 w - - 5 30", Move{CAPTURE, E3, D5},
                   "4k3/8/8/3N4/8/8/8/4K3 b - - 0 30");
    }
    SECTION("black knight takes knight") {
        check_move("4k3/8/8/3n4/8/4N3/8/4K3 b - - 5 30", Move{CAPTURE, D5, E3},
                   "4k3/8/8/8/8/4n3/8/4K3 w - - 0 31");
    }
}

TEST_CASE("make_move: en passant removes the pawn behind the target", "[board][make_move]") {
    SECTION("white") {
        check_move("rnbqkbnr/ppp1p1pp/8/3pPp2/8/8/PPPP1PPP/RNBQKBNR w KQkq d6 0 3",
                   Move{EN_PASSANT, E5, D6},
                   "rnbqkbnr/ppp1p1pp/3P4/5p2/8/8/PPPP1PPP/RNBQKBNR b KQkq - 0 3");
    }
    SECTION("black") {
        check_move("rnbqkbnr/pppp1ppp/8/8/3Pp3/8/PPP1PPPP/RNBQKBNR b KQkq d3 0 2",
                   Move{EN_PASSANT, E4, D3},
                   "rnbqkbnr/pppp1ppp/8/8/8/3p4/PPP1PPPP/RNBQKBNR w KQkq - 0 3");
    }
}

TEST_CASE("make_move: castling moves the rook and removes both rights", "[board][make_move]") {
    SECTION("white king side") {
        check_move(castling_fen_white, Move{KING_SIDE_CASTLE, E1, G1},
                   "r3k2r/8/8/8/8/8/8/R4RK1 b kq - 1 1");
    }
    SECTION("white queen side") {
        check_move(castling_fen_white, Move{QUEEN_SIDE_CASTLE, E1, C1},
                   "r3k2r/8/8/8/8/8/8/2KR3R b kq - 1 1");
    }
    SECTION("black king side") {
        check_move(castling_fen_black, Move{KING_SIDE_CASTLE, E8, G8},
                   "r4rk1/8/8/8/8/8/8/R3K2R w KQ - 1 2");
    }
    SECTION("black queen side") {
        check_move(castling_fen_black, Move{QUEEN_SIDE_CASTLE, E8, C8},
                   "2kr3r/8/8/8/8/8/8/R3K2R w KQ - 1 2");
    }
}

TEST_CASE("make_move: king and rook moves remove castling rights", "[board][make_move]") {
    SECTION("king move removes both rights") {
        check_move(castling_fen_white, Move{0, E1, E2}, "r3k2r/8/8/8/8/8/4K3/R6R b kq - 1 1");
    }
    SECTION("h1 rook move removes king side") {
        check_move(castling_fen_white, Move{0, H1, H2}, "r3k2r/8/8/8/8/8/7R/R3K3 b Qkq - 1 1");
    }
    SECTION("a1 rook move removes queen side") {
        check_move(castling_fen_white, Move{0, A1, A2}, "r3k2r/8/8/8/8/8/R7/4K2R b Kkq - 1 1");
    }
    SECTION("capturing a rook on its home square removes the opponent's right") {
        check_move(castling_fen_white, Move{CAPTURE, H1, H8},
                   "r3k2R/8/8/8/8/8/8/R3K3 b Qq - 0 1");
    }
    SECTION("black rook captures on a1") {
        check_move(castling_fen_black, Move{CAPTURE, A8, A1},
                   "4k2r/8/8/8/8/8/8/r3K2R w Kk - 0 2");
    }
}

TEST_CASE("make_move: promotions", "[board][make_move]") {
    SECTION("white promotes to queen") {
        check_move("8/4P3/8/8/8/8/8/k6K w - - 3 50", Move{QUEEN_PROMOTION, E7, E8},
                   "4Q3/8/8/8/8/8/8/k6K b - - 0 50");
    }
    SECTION("white promotes to knight") {
        check_move("8/4P3/8/8/8/8/8/k6K w - - 3 50", Move{KNIGHT_PROMOTION, E7, E8},
                   "4N3/8/8/8/8/8/8/k6K b - - 0 50");
    }
    SECTION("capture-promotion to the captured piece's type") {
        check_move("2r1k3/3P4/8/8/8/8/8/4K3 w - - 0 1", Move{ROOK_PROMOTION | CAPTURE, D7, C8},
                   "2R1k3/8/8/8/8/8/8/4K3 b - - 0 1");
    }
    SECTION("capture-promotion to queen") {
        check_move("2r1k3/3P4/8/8/8/8/8/4K3 w - - 0 1", Move{QUEEN_PROMOTION | CAPTURE, D7, C8},
                   "2Q1k3/8/8/8/8/8/8/4K3 b - - 0 1");
    }
    SECTION("black promotes to queen") {
        check_move("4k3/8/8/8/8/8/1p6/4K3 b - - 0 1", Move{QUEEN_PROMOTION, B2, B1},
                   "4k3/8/8/8/8/8/8/1q2K3 w - - 0 2");
    }
    SECTION("black capture-promotion on a1 removes white's queen side right") {
        check_move("4k3/8/8/8/8/8/1p6/R3K3 b Q - 0 1", Move{KNIGHT_PROMOTION | CAPTURE, B2, A1},
                   "4k3/8/8/8/8/8/8/n3K3 w - - 0 2");
    }
}
