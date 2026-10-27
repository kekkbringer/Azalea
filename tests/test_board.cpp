#include "board.hpp"

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include <bit>
#include <cstdint>
#include <initializer_list>
#include <string>

using azalea::Board;

namespace {

// Builds a bitboard with the given squares set.
std::uint64_t squares(std::initializer_list<int> list) {
    std::uint64_t bb = 0;
    for (const int sq : list) {
        bb |= 1ULL << sq;
    }
    return bb;
}

constexpr const char* start_fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

constexpr int all_castle_rights =
    azalea::WHITE_SHORT | azalea::WHITE_LONG | azalea::BLACK_SHORT | azalea::BLACK_LONG;

} // namespace

TEST_CASE("Square names convert to indices and back", "[board][square]") {
    CHECK(azalea::alg_to_int("a1") == azalea::A1);
    CHECK(azalea::alg_to_int("h1") == azalea::H1);
    CHECK(azalea::alg_to_int("e4") == azalea::E4);
    CHECK(azalea::alg_to_int("a8") == azalea::A8);
    CHECK(azalea::alg_to_int("h8") == azalea::H8);

    for (int sq = 0; sq < 64; ++sq) {
        INFO("square = " << sq);
        CHECK(azalea::alg_to_int(azalea::int_to_alg(sq)) == sq);
    }
}

TEST_CASE("Start position is parsed correctly", "[board][fen]") {
    const Board board{start_fen};

    using namespace azalea;
    CHECK(board.white_pieces == 0x0000'0000'0000'FFFFULL);
    CHECK(board.black_pieces == 0xFFFF'0000'0000'0000ULL);
    CHECK(board.pawns == 0x00FF'0000'0000'FF00ULL);
    CHECK(board.knights == squares({B1, G1, B8, G8}));
    CHECK(board.bishops == squares({C1, F1, C8, F8}));
    CHECK(board.rooks == squares({A1, H1, A8, H8}));
    CHECK(board.queens == squares({D1, D8}));
    CHECK(board.kings == squares({E1, E8}));

    CHECK(board.white_to_move);
    CHECK(board.castle_rights() == all_castle_rights);
    CHECK(board.en_passant_target() == -1);
    CHECK(board.halfmove_clock() == 0);
    CHECK(board.fullmove_number == 1);
}

TEST_CASE("Pieces land on the right squares", "[board][fen]") {
    // Kiwipete: an asymmetric position, so a shifted rank or file shows up.
    const Board board{"r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1"};

    using namespace azalea;
    CHECK(board.kings == squares({E1, E8}));
    CHECK(board.queens == squares({F3, E7}));
    CHECK(board.rooks == squares({A1, H1, A8, H8}));
    CHECK(board.bishops == squares({D2, E2, A6, G7}));
    CHECK(board.knights == squares({C3, E5, B6, F6}));
    CHECK(board.pawns == squares({A2, B2, C2, F2, G2, H2, E4, D5, B4, H3, A7, C7, D7, F7, E6,
                                  G6}));
    CHECK(board.white_pieces == squares({A1, E1, H1, A2, B2, C2, D2, E2, F2, G2, H2, C3, F3, E4,
                                         D5, E5}));
    CHECK(board.black_pieces == squares({A8, E8, H8, A7, C7, D7, E7, F7, G7, A6, B6, E6, F6, G6,
                                         B4, H3}));
}

TEST_CASE("Side to move is parsed", "[board][fen]") {
    CHECK(Board{"8/8/8/8/8/8/8/K6k w - - 0 1"}.white_to_move);
    CHECK_FALSE(Board{"8/8/8/8/8/8/8/K6k b - - 0 1"}.white_to_move);
}

TEST_CASE("Castling rights are parsed", "[board][fen]") {
    const auto rights = [](const char* field) {
        return Board{std::string{"r3k2r/8/8/8/8/8/8/R3K2R w "} + field + " - 0 1"}.castle_rights();
    };

    CHECK(rights("KQkq") == all_castle_rights);
    CHECK(rights("-") == 0);
    CHECK(rights("K") == azalea::WHITE_SHORT);
    CHECK(rights("Q") == azalea::WHITE_LONG);
    CHECK(rights("k") == azalea::BLACK_SHORT);
    CHECK(rights("q") == azalea::BLACK_LONG);
    CHECK(rights("Kq") == (azalea::WHITE_SHORT | azalea::BLACK_LONG));
    CHECK(rights("Qk") == (azalea::WHITE_LONG | azalea::BLACK_SHORT));
}

TEST_CASE("En passant target is parsed", "[board][fen]") {
    CHECK(Board{start_fen}.en_passant_target() == -1);
    CHECK(Board{"4k3/8/8/8/3pP3/8/8/4K3 b - e3 0 1"}.en_passant_target() == azalea::E3);
    CHECK(Board{"rnbqkbnr/ppp1p1pp/8/3pPp2/8/8/PPPP1PPP/RNBQKBNR w KQkq f6 0 3"}
              .en_passant_target() == azalea::F6);
}

TEST_CASE("En passant target is dropped if no pawn can capture", "[board][fen]") {
    // Many GUIs write the square after every double push. Keeping it would make the
    // position differ from the same position reached without it.
    CHECK(Board{"rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1"}
              .en_passant_target() == -1);
    CHECK(Board{"rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1"}.hash()
          == Board{"rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq - 0 1"}.hash());
    // An own pawn next to the square can't capture either.
    CHECK(Board{"4k3/8/8/8/3PP3/8/8/4K3 b - e3 0 1"}.en_passant_target() == -1);
}

TEST_CASE("Move counters are parsed", "[board][fen]") {
    const Board board{"r3k2r/8/8/8/8/8/8/R3K2R b Kq - 47 112"};
    CHECK(board.halfmove_clock() == 47);
    CHECK(board.fullmove_number == 112);
}

TEST_CASE("Parsed bitboards are consistent", "[board][fen]") {
    const std::string fen = GENERATE(
        start_fen, "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1",
        "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1",
        "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1",
        "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8");
    INFO("fen = " << fen);
    const Board board{fen};

    // No square belongs to both colors.
    CHECK((board.white_pieces & board.black_pieces) == 0);

    // No square holds two piece types.
    const std::uint64_t types[] = {board.pawns, board.knights, board.bishops,
                                   board.rooks, board.queens,  board.kings};
    for (std::size_t i = 0; i < 6; ++i) {
        for (std::size_t j = i + 1; j < 6; ++j) {
            CHECK((types[i] & types[j]) == 0);
        }
    }

    // Every occupied square has a piece type and vice versa.
    const std::uint64_t all_types = board.pawns | board.knights | board.bishops | board.rooks |
                                    board.queens | board.kings;
    CHECK(all_types == (board.white_pieces | board.black_pieces));

    // Exactly one king per side.
    CHECK(std::popcount(board.kings & board.white_pieces) == 1);
    CHECK(std::popcount(board.kings & board.black_pieces) == 1);
}

TEST_CASE("to_fen() reproduces the parsed FEN", "[board][fen]") {
    const std::string fen = GENERATE(
        // start position and the standard perft test positions
        start_fen, "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1",
        "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1",
        "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1",
        "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8",
        "r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10",
        // en passant targets for both sides (only kept if a pawn can capture)
        "rnbqkbnr/ppp1pppp/8/8/3pP3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 3",
        "rnbqkbnr/ppp1p1pp/8/3pPp2/8/8/PPPP1PPP/RNBQKBNR w KQkq f6 0 3",
        // black to move, partial castling rights, larger counters
        "rnbqkbnr/pp1ppppp/8/2p5/4P3/5N2/PPPP1PPP/RNBQKB1R b KQkq - 1 2",
        "r3k2r/8/8/8/8/8/8/R3K2R b Kq - 47 112",
        // pieces on the a- and h-files and in the corners
        "k7/8/8/8/8/8/8/7K w - - 0 1", "7k/P7/8/8/8/8/7p/K7 b - - 0 60");
    INFO("fen = " << fen);
    CHECK(Board{fen}.to_fen() == fen);
}
