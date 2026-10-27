#include "board.hpp"
#include "movegeneration.hpp"

#include <catch2/catch_test_macros.hpp>

#include <initializer_list>
#include <string_view>

using azalea::Board;
using azalea::Move;
using azalea::Movelist;

namespace {

constexpr const char* start = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

void play(Board& board, std::initializer_list<std::string_view> moves) {
    for (const auto uci : moves) {
        Movelist mlist;
        azalea::generate_legal_moves(board, mlist);
        bool found = false;
        for (const Move m : mlist) {
            if (azalea::to_uci(m) == uci) {
                board.make_move(m);
                found = true;
                break;
            }
        }
        INFO("move: " << uci);
        REQUIRE(found);
    }
}

} // namespace

TEST_CASE("is_repetition: a fresh position is not a repetition", "[board][repetition]") {
    CHECK_FALSE(Board{start}.is_repetition());
    // A halfmove clock larger than the history must not read before the first entry.
    CHECK_FALSE(Board{"4k3/8/8/8/8/8/8/R3K3 w - - 37 60"}.is_repetition());
}

TEST_CASE("is_repetition: knights out and back repeat the start position", "[board][repetition]") {
    Board board{start};
    play(board, {"g1f3"});
    CHECK_FALSE(board.is_repetition());
    play(board, {"g8f6"});
    CHECK_FALSE(board.is_repetition());
    play(board, {"f3g1"});
    CHECK_FALSE(board.is_repetition());
    play(board, {"f6g8"});
    CHECK(board.is_repetition());

    // A third occurrence is a repetition too.
    play(board, {"g1f3", "g8f6", "f3g1", "f6g8"});
    CHECK(board.is_repetition());
}

TEST_CASE("is_repetition: the position right after an irreversible move can repeat",
          "[board][repetition]") {
    // 1. e3 is a single push, so no en passant square is set and the position after it
    // (black to move) is reached again after 1... Nf6 2. Nf3 Ng8 3. Ng1.
    Board board{start};
    play(board, {"e2e3", "g8f6", "g1f3", "f6g8", "f3g1"});
    CHECK(board.is_repetition());
}

TEST_CASE("is_repetition: an unusable en passant square doesn't break a repetition",
          "[board][repetition]") {
    // After 1. e4 no black pawn can capture on e3, so the position after 1. e4 comes
    // back after 1... Nf6 2. Nf3 Ng8 3. Ng1.
    Board board{start};
    play(board, {"e2e4", "g8f6", "g1f3", "f6g8", "f3g1"});
    CHECK(board.is_repetition());
}

TEST_CASE("is_repetition: a usable en passant square is part of the position",
          "[board][repetition]") {
    // After e4 black could capture d4xe3, after the king moves that right is gone, so the
    // same placement with black to move is a different position.
    Board board{"4k3/8/8/8/3p4/8/4P3/4K3 w - - 0 1"};
    play(board, {"e2e4", "e8e7", "e1d1", "e7e8", "d1e1"});
    CHECK_FALSE(board.is_repetition());
    // From here on the en passant right is gone for good, so the next round trip repeats.
    play(board, {"e8e7", "e1d1", "e7e8", "d1e1"});
    CHECK(board.is_repetition());
}

TEST_CASE("is_repetition: the side to move must match", "[board][repetition]") {
    // The white king walks a triangle and the black king steps out and back, so the
    // starting placement comes back, but with black to move instead of white.
    Board board{"7k/8/8/8/8/8/8/K7 w - - 0 1"};
    play(board, {"a1a2", "h8h7", "a2b1", "h7h8", "b1a1"});
    CHECK_FALSE(board.is_repetition());
    // Two more moves each bring back the same placement with black to move again.
    play(board, {"h8g8", "a1a2", "g8h8", "a2a1"});
    CHECK(board.is_repetition());
}

TEST_CASE("is_repetition: castling rights are part of the position", "[board][repetition]") {
    Board board{"r3k2r/8/8/8/8/8/8/R3K2R w KQkq - 0 1"};
    // Same placement as the start, but both sides have lost their castling rights.
    play(board, {"e1f1", "e8f8", "f1e1", "f8e8"});
    CHECK_FALSE(board.is_repetition());
    // Now the rights stay the same, so the next round trip repeats.
    play(board, {"e1f1", "e8f8", "f1e1", "f8e8"});
    CHECK(board.is_repetition());
}

TEST_CASE("is_repetition: unmake_move restores the answer", "[board][repetition]") {
    Board board{start};
    play(board, {"g1f3", "g8f6", "f3g1"});

    Movelist mlist;
    azalea::generate_legal_moves(board, mlist);
    for (const Move m : mlist) {
        if (azalea::to_uci(m) != "f6g8") continue;
        board.make_move(m);
        CHECK(board.is_repetition());
        board.unmake_move(m);
    }
    CHECK_FALSE(board.is_repetition());
}
