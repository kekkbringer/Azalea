#include "board.hpp"
#include "eval.hpp"
#include "movegeneration.hpp"

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include <cctype>
#include <sstream>
#include <string>
#include <vector>

using azalea::Board;

namespace {

// Mirrors a FEN vertically and swaps the colours: rank 1 becomes rank 8, white pieces become
// black pieces, the side to move flips and so do castling rights and the en passant square.
// The mirrored position is the same position seen from the other side, so eval() (which scores
// from the side to move's view) must return exactly the same value for both.
std::string mirror_fen(const std::string& fen) {
    std::istringstream iss{fen};
    std::string placement, side, castling, ep, halfmove, fullmove;
    iss >> placement >> side >> castling >> ep >> halfmove >> fullmove;

    auto swap_case = [](char c) {
        const auto u = static_cast<unsigned char>(c);
        return std::isupper(u) ? static_cast<char>(std::tolower(u))
                               : static_cast<char>(std::toupper(u));
    };

    std::vector<std::string> ranks;
    std::istringstream ranks_stream{placement};
    for (std::string rank; std::getline(ranks_stream, rank, '/');) {
        for (char& c : rank) c = swap_case(c);
        ranks.push_back(rank);
    }
    std::string mirrored_placement;
    for (auto it = ranks.rbegin(); it != ranks.rend(); ++it) {
        if (!mirrored_placement.empty()) mirrored_placement += '/';
        mirrored_placement += *it;
    }

    // Castling rights are listed as KQkq; after swapping case the order must be restored.
    std::string mirrored_castling;
    if (castling == "-") {
        mirrored_castling = "-";
    } else {
        for (const char c : {'K', 'Q', 'k', 'q'})
            if (castling.find(swap_case(c)) != std::string::npos) mirrored_castling += c;
    }

    std::string mirrored_ep = ep;
    if (ep != "-") mirrored_ep[1] = static_cast<char>('1' + ('8' - ep[1]));

    return mirrored_placement + ' ' + (side == "w" ? "b" : "w") + ' ' + mirrored_castling + ' '
         + mirrored_ep + ' ' + halfmove + ' ' + fullmove;
}

// Checks the symmetry for every position in the legal move tree below `board`.
void find_asymmetries(Board& board, int depth, std::vector<std::string>& failures) {
    const std::string fen = board.to_fen();
    const int original = azalea::eval(board);
    const int mirrored = azalea::eval(Board{mirror_fen(fen)});
    if (original != mirrored && failures.size() < 10)
        failures.push_back(fen + "  eval " + std::to_string(original) + " vs mirrored "
                           + std::to_string(mirrored));

    if (depth == 0) return;
    azalea::Movelist moves;
    azalea::generate_legal_moves(board, moves);
    for (const auto& m : moves) {
        board.make_move(m);
        find_asymmetries(board, depth - 1, failures);
        board.unmake_move(m);
    }
}

} // namespace

TEST_CASE("mirror_fen: mirrors placement, side, castling and en passant", "[eval]") {
    // Position 4 and its mirrored version are both standard perft positions.
    CHECK(mirror_fen("r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1")
          == "r2q1rk1/pP1p2pp/Q4n2/bbp1p3/Np6/1B3NBn/pPPP1PPP/R3K2R b KQ - 0 1");
    CHECK(mirror_fen("rnbqkbnr/ppp1pppp/8/8/3pP3/8/PPPP1PPP/RNBQKBNR b Kq e3 0 3")
          == "rnbqkbnr/pppp1ppp/8/3Pp3/8/8/PPP1PPPP/RNBQKBNR w Qk e6 0 3");

    const std::string fen = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1";
    CHECK(mirror_fen(mirror_fen(fen)) == fen);
}

TEST_CASE("eval is colour symmetric", "[eval]") {
    struct Case {
        const char* fen;
        int depth;
    };
    const Case c = GENERATE(values<Case>({
        // perft positions: openings, middlegames, castling, en passant and promotions
        {"rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", 3},
        {"r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1", 2},
        {"8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1", 3},
        {"r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1", 2},
        {"rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8", 2},
        {"r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10", 2},
        // endgames, so the late-phase tables get covered as well
        {"8/8/4k3/8/8/4K3/4P3/8 w - - 0 1", 3},
        {"8/5pk1/6p1/8/3R4/6P1/5PK1/8 b - - 0 1", 3},
        {"8/8/3k4/8/2B5/8/2N1K3/8 w - - 0 1", 2},
        {"6k1/8/8/8/8/8/1q6/4K3 w - - 0 1", 3},
        {"8/1P6/8/8/8/2k5/6p1/K7 w - - 0 1", 3},
    }));

    INFO(c.fen);
    Board board{c.fen};
    std::vector<std::string> failures;
    find_asymmetries(board, c.depth, failures);
    for (const auto& f : failures) UNSCOPED_INFO(f);
    CHECK(failures.empty());
}
