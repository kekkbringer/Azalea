#include "board.hpp"
#include "movegeneration.hpp"

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include <sstream>
#include <string>
#include <vector>

using azalea::Board;
using azalea::Move;
using azalea::Movelist;
using azalea::PositionInfo;

namespace {

bool same_info(const PositionInfo& a, const PositionInfo& b) {
    return a.hash == b.hash and a.en_passant_target == b.en_passant_target
       and a.castle_rights == b.castle_rights and a.halfmove_clock == b.halfmove_clock
       and a.captured == b.captured;
}

// Describes the first difference between two boards, or returns "" if they are identical.
// Compares the raw state, not just the FEN: a stray bit in a color bitboard or a
// leftover history entry wouldn't show up in to_fen().
std::string first_difference(const Board& expected, const Board& actual) {
    std::ostringstream out;
    out << std::hex;
    const auto bb = [&](const char* name, std::uint64_t e, std::uint64_t a) {
        if (out.tellp() == 0 and e != a) out << name << ": expected 0x" << e << ", got 0x" << a;
    };
    bb("white_pieces", expected.white_pieces, actual.white_pieces);
    bb("black_pieces", expected.black_pieces, actual.black_pieces);
    bb("pawns", expected.pawns, actual.pawns);
    bb("knights", expected.knights, actual.knights);
    bb("bishops", expected.bishops, actual.bishops);
    bb("rooks", expected.rooks, actual.rooks);
    bb("queens", expected.queens, actual.queens);
    bb("kings", expected.kings, actual.kings);
    out << std::dec;

    if (out.tellp() != 0) return out.str();
    if (expected.white_to_move != actual.white_to_move) return "white_to_move differs";
    if (expected.fullmove_number != actual.fullmove_number) {
        return "fullmove_number: expected " + std::to_string(expected.fullmove_number) + ", got "
             + std::to_string(actual.fullmove_number);
    }
    if (expected.position_history.size() != actual.position_history.size()) {
        return "history size: expected " + std::to_string(expected.position_history.size())
             + ", got " + std::to_string(actual.position_history.size());
    }
    for (std::size_t i = 0; i < expected.position_history.size(); ++i) {
        if (not same_info(expected.position_history[i], actual.position_history[i])) {
            return "history entry " + std::to_string(i) + " differs";
        }
    }
    return "";
}

// Plays every legal move sequence up to `depth` on one board with make/unmake and checks
// after each unmake_move that the board is exactly what it was before make_move.
// Nested make/unmake pairs also check that unmaking restores the history correctly.
// Counts failures instead of CHECKing each one so a broken unmake doesn't flood the output.
void find_round_trip_failures(Board& board, int depth, int& failures) {
    if (depth == 0) return;

    Movelist mlist;
    azalea::generate_legal_moves(board, mlist);

    for (const Move m : mlist) {
        const Board before = board;
        board.make_move(m);
        find_round_trip_failures(board, depth - 1, failures);
        board.unmake_move(m);

        const std::string diff = first_difference(before, board);
        if (not diff.empty()) {
            if (failures == 0) {
                UNSCOPED_INFO("first failure: " << before.to_fen() << " + " << azalea::to_uci(m)
                                                << ": " << diff);
            }
            ++failures;
            board = before; // continue from a correct board so later moves are tested too
        }
    }
}

} // namespace

TEST_CASE("unmake_move restores the board exactly", "[board][unmake_move]") {
    // The perft positions cover castling, loss of castling rights, en passant and promotions.
    const auto [name, fen, depth] = GENERATE(table<const char*, const char*, int>({
        {"start", "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", 3},
        {"kiwipete", "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1", 2},
        {"position 3", "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1", 3},
        {"position 4", "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1", 2},
        {"position 5", "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8", 2},
        {"position 6", "r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10", 2},
        // small positions where the tricky moves are the first ones searched
        {"en passant, white", "4k3/8/8/2pP4/8/8/8/4K3 w - c6 0 1", 1},
        {"en passant, black", "4k3/8/8/8/2Pp4/8/8/4K3 b - c3 0 1", 1},
        {"promotions", "1r2k3/P7/8/8/8/8/p7/1R2K3 w - - 0 1", 2},
        {"castling", "r3k2r/8/8/8/8/8/8/R3K2R w KQkq - 0 1", 2},
    }));

    INFO(name << ", depth " << depth << ": " << fen);
    Board board{fen};
    int failures = 0;
    find_round_trip_failures(board, depth, failures);
    CHECK(failures == 0);
}

TEST_CASE("unmaking a line of moves in reverse order returns the start position",
          "[board][unmake_move]") {
    Board board{"r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1"};
    const std::string fen = board.to_fen();

    std::vector<Move> played;
    for (int i = 0; i < 6; ++i) {
        Movelist mlist;
        azalea::generate_legal_moves(board, mlist);
        REQUIRE(mlist.size() > 0);
        const Move m = mlist.moves[0];
        board.make_move(m);
        played.push_back(m);
    }
    for (auto it = played.rbegin(); it != played.rend(); ++it) board.unmake_move(*it);

    CHECK(board.to_fen() == fen);
    CHECK(board.position_history.size() == Board{fen}.position_history.size());
}
