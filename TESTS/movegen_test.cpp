#include <catch2/catch.hpp>

#define CATCH_CONFIG_NOSTDOUT

#include <vector>

#include "board.hpp"
#include "move.hpp"
#include "movegenerator.hpp"
#include "fen.hpp"

TEST_CASE("testing movegen...", "[movegen]") {
    std::vector<Move> movelist;
    SECTION("king") {
	const Board b
	    = fen("8/8/8/8/3K4/8/8/8 w - - 0 1");
	generateLegalMoves(b, movelist);
	REQUIRE(movelist.size()==8);

	const Board b2
	    = fen("8/8/8/8/8/8/8/4K3 w - - 0 1");
	generateLegalMoves(b2, movelist);
	REQUIRE(movelist.size()==5);

	const Board b3
	    = fen("8/8/8/K7/8/8/8/8 w - - 0 1");
	generateLegalMoves(b3, movelist);
	REQUIRE(movelist.size()==5);

	const Board b4
	    = fen("4K3/8/8/8/8/8/8/8 w - - 0 1");
	generateLegalMoves(b4, movelist);
	REQUIRE(movelist.size()==5);

	const Board b5
	    = fen("8/8/8/8/8/8/7K/8 w - - 0 1");
	generateLegalMoves(b5, movelist);
	REQUIRE(movelist.size()==5);

	const Board b6
	    = fen("8/8/8/8/8/8/8/7K w - - 0 1");
	generateLegalMoves(b6, movelist);
	REQUIRE(movelist.size()==3);

	const Board b7
	    = fen("8/8/8/8/8/8/8/K7 w - - 0 1");
	generateLegalMoves(b7, movelist);
	REQUIRE(movelist.size()==3);

	const Board b8
	    = fen("K7/8/8/8/8/8/8/8 w - - 0 1");
	generateLegalMoves(b8, movelist);
	REQUIRE(movelist.size()==3);

	const Board b9
	    = fen("7k/8/8/8/8/8/8/8 b - - 0 1");
	generateLegalMoves(b9, movelist);
	REQUIRE(movelist.size()==3);
    }

    SECTION("king with danger") {

    }
}
