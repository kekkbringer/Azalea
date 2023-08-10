#include "move.hpp"
#include "board.hpp"
#include "movegenerator.hpp"

#include <vector>

unsigned long long int perft(const GameState& gs, int depth) {
    unsigned long long int nodes = 0;

    std::vector<Move> ml;
    generateLegalMoves(gs, ml);
    const int nMoves = ml.size();

    if (depth == 1) return (unsigned long long int)nMoves;

    for (const auto& m: ml) {
	auto dummy = gs;
	dummy.makeMove(m);
	nodes += perft(dummy, depth-1);
    }

    return nodes;
}
