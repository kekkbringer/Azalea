#include "perft.hpp"

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include <cstdint>
#include <string>

using azalea::Board;

namespace {

struct PerftCase {
    const char* name;
    const char* fen;
    int depth;
    std::uint64_t nodes;
};

// Standard test positions and node counts from https://www.chessprogramming.org/Perft_Results
constexpr const char* start = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
constexpr const char* kiwipete =
    "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1";
constexpr const char* position3 = "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1";
constexpr const char* position4 =
    "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1";
constexpr const char* position4_mirrored =
    "r2q1rk1/pP1p2pp/Q4n2/bbp1p3/Np6/1B3NBn/pPPP1PPP/R3K2R b KQ - 0 1";
constexpr const char* position5 = "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8";
constexpr const char* position6 =
    "r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10";

} // namespace

TEST_CASE("perft: standard positions", "[perft]") {
    // Kept below ~100k nodes per case so the suite stays fast in debug builds.
    const PerftCase c = GENERATE(values<PerftCase>({
        {"start", start, 1, 20},
        {"start", start, 2, 400},
        {"start", start, 3, 8'902},
        {"kiwipete", kiwipete, 1, 48},
        {"kiwipete", kiwipete, 2, 2'039},
        {"kiwipete", kiwipete, 3, 97'862},
        {"position 3", position3, 1, 14},
        {"position 3", position3, 2, 191},
        {"position 3", position3, 3, 2'812},
        {"position 3", position3, 4, 43'238},
        {"position 4", position4, 1, 6},
        {"position 4", position4, 2, 264},
        {"position 4", position4, 3, 9'467},
        {"position 4 mirrored", position4_mirrored, 1, 6},
        {"position 4 mirrored", position4_mirrored, 2, 264},
        {"position 4 mirrored", position4_mirrored, 3, 9'467},
        {"position 5", position5, 1, 44},
        {"position 5", position5, 2, 1'486},
        {"position 5", position5, 3, 62'379},
        {"position 6", position6, 1, 46},
        {"position 6", position6, 2, 2'079},
        {"position 6", position6, 3, 89'890},
    }));

    INFO(c.name << ", depth " << c.depth << ": " << c.fen);
    Board board{c.fen};
    const std::string fen_before = board.to_fen();
    CHECK(azalea::perft(board, c.depth) == c.nodes);
    CHECK(board.to_fen() == fen_before); // make/unmake must leave the position unchanged
}

// Hidden by default (the leading "." in the tag). Run with:
//   build/release/tests/azalea_tests "[slow]"
TEST_CASE("perft: deep standard positions", "[.slow][perft]") {
    const PerftCase c = GENERATE(values<PerftCase>({
        {"start", start, 4, 197'281},
        {"start", start, 5, 4'865'609},
        {"kiwipete", kiwipete, 4, 4'085'603},
        {"position 3", position3, 5, 674'624},
        {"position 4", position4, 4, 422'333},
        {"position 4 mirrored", position4_mirrored, 4, 422'333},
        {"position 5", position5, 4, 2'103'487},
        {"position 6", position6, 4, 3'894'594},
    }));

    INFO(c.name << ", depth " << c.depth << ": " << c.fen);
    Board board{c.fen};
    const std::string fen_before = board.to_fen();
    CHECK(azalea::perft(board, c.depth) == c.nodes);
    CHECK(board.to_fen() == fen_before); // make/unmake must leave the position unchanged
}

TEST_CASE("perft_divide: one entry per legal root move", "[perft]") {
    Board board{start};

    const auto depth1 = azalea::perft_divide(board, 1);
    CHECK(depth1.size() == 20);
    for (const auto& entry : depth1) {
        INFO("move: " << azalea::to_uci(entry.move));
        CHECK(entry.nodes == 1);
    }

    // After any white first move, black has exactly 20 replies.
    const auto depth2 = azalea::perft_divide(board, 2);
    CHECK(depth2.size() == 20);
    for (const auto& entry : depth2) {
        INFO("move: " << azalea::to_uci(entry.move));
        CHECK(entry.nodes == 20);
    }
}

TEST_CASE("perft_divide entries add up to perft", "[perft]") {
    const char* fen = GENERATE(start, kiwipete, position3, position4, position5, position6);
    INFO("fen: " << fen);
    Board board{fen};

    std::uint64_t total = 0;
    for (const auto& entry : azalea::perft_divide(board, 2)) {
        total += entry.nodes;
    }
    CHECK(total == azalea::perft(board, 2));
}
