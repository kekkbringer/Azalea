#include "board.hpp"
#include "movegeneration.hpp"

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include <cstdint>
#include <string>
#include <string_view>

using azalea::Board;
using azalea::Move;
using azalea::Movelist;

namespace {

// The hash computed from scratch by the FEN constructor is the reference.
std::uint64_t reference_hash(const Board& board) {
    return Board{board.to_fen()}.hash();
}

// Walks every legal move sequence up to `depth` and checks that the incrementally
// updated hash matches the hash recomputed from the FEN after each move.
// Counts mismatches instead of CHECKing each one so a broken make_move doesn't flood the output.
void find_hash_mismatches(Board& board, int depth, int& mismatches) {
    if (depth == 0) return;

    Movelist mlist;
    azalea::generate_legal_moves(board, mlist);

    for (const Move m : mlist) {
        Board copy = board;
        copy.make_move(m);
        if (copy.hash() != reference_hash(copy)) {
            if (mismatches == 0) {
                UNSCOPED_INFO("first mismatch: " << board.to_fen() << " + " << azalea::to_uci(m));
            }
            ++mismatches;
            continue; // don't descend from a broken hash, it only repeats the same error
        }
        find_hash_mismatches(copy, depth - 1, mismatches);
    }
}

Board play(std::string_view fen, std::initializer_list<std::string_view> moves) {
    Board board{fen};
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
    return board;
}

constexpr const char* start = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

} // namespace

TEST_CASE("zobrist: incremental hash matches the hash from the FEN", "[zobrist]") {
    // The perft positions cover castling, loss of castling rights, en passant and promotions.
    const auto [name, fen, depth] = GENERATE(table<const char*, const char*, int>({
        {"start", start, 3},
        {"kiwipete", "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1", 2},
        {"position 3", "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1", 3},
        {"position 4", "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1", 2},
        {"position 5", "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8", 2},
        {"position 6", "r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10", 2},
        {"en passant", "4k3/8/8/2pP4/8/8/8/4K3 w - c6 0 1", 2},
        {"promotions", "1r2k3/P7/8/8/8/8/p7/1R2K3 w - - 0 1", 2},
    }));

    INFO(name << ", depth " << depth << ": " << fen);
    Board board{fen};
    CHECK(board.hash() == reference_hash(board));
    int mismatches = 0;
    find_hash_mismatches(board, depth, mismatches);
    CHECK(mismatches == 0);
}

TEST_CASE("zobrist: transpositions have the same hash", "[zobrist]") {
    SECTION("knight moves in a different order") {
        const Board a = play(start, {"g1f3", "g8f6", "b1c3", "b8c6"});
        const Board b = play(start, {"b1c3", "b8c6", "g1f3", "g8f6"});
        CHECK(a.hash() == b.hash());
    }
    SECTION("knights going out and back give the start position") {
        const Board a = play(start, {"g1f3", "g8f6", "f3g1", "f6g8"});
        CHECK(a.hash() == Board{start}.hash());
    }
}

TEST_CASE("zobrist: positions that differ only in state have different hashes", "[zobrist]") {
    SECTION("side to move") {
        CHECK(Board{"4k3/8/8/8/8/8/8/4K3 w - - 0 1"}.hash()
              != Board{"4k3/8/8/8/8/8/8/4K3 b - - 0 1"}.hash());
    }
    SECTION("castling rights") {
        CHECK(Board{"r3k2r/8/8/8/8/8/8/R3K2R w KQkq - 0 1"}.hash()
              != Board{"r3k2r/8/8/8/8/8/8/R3K2R w Kkq - 0 1"}.hash());
    }
    SECTION("en passant square") {
        CHECK(Board{"4k3/8/8/2pP4/8/8/8/4K3 w - c6 0 1"}.hash()
              != Board{"4k3/8/8/2pP4/8/8/8/4K3 w - - 0 1"}.hash());
    }
    SECTION("losing castling rights by moving the king and back") {
        // Same pieces, same side to move, but no castling rights any more.
        const Board a = play("r3k2r/8/8/8/8/8/8/R3K2R w KQkq - 0 1", {"e1f1", "e8f8", "f1e1", "f8e8"});
        CHECK(a.hash() == Board{"r3k2r/8/8/8/8/8/8/R3K2R w - - 4 3"}.hash());
        CHECK(a.hash() != Board{"r3k2r/8/8/8/8/8/8/R3K2R w KQkq - 0 1"}.hash());
    }
}
