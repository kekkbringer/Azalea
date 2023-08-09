#include <catch2/catch.hpp>

#define CATCH_CONFIG_NOSTDOUT

#include <algorithm>

#include "board.hpp"
#include "fen.hpp"

TEST_CASE("Testing board representation...", "[board]") {
    SECTION("side to move") {
	const auto bw
	    = fen("8/8/8/8/8/8/8/8 w - - 0 1");
	REQUIRE(bw.whiteToMove);
	const auto bb
	    = fen("8/8/8/8/8/8/8/8 b - - 0 1");
	REQUIRE(!bb.whiteToMove);
    }
    SECTION("castle rights") {
	const auto b1
	    = fen("8/8/8/8/8/8/8/8 w K - 0 1");
	REQUIRE(b1.whiteShort);
	REQUIRE(!b1.blackShort);
	REQUIRE(!b1.whiteLong);
	REQUIRE(!b1.blackLong);
	const auto b2
	    = fen("8/8/8/8/8/8/8/8 w k - 0 1");
	REQUIRE(!b2.whiteShort);
	REQUIRE(b2.blackShort);
	REQUIRE(!b2.whiteLong);
	REQUIRE(!b2.blackLong);
	const auto b3
	    = fen("8/8/8/8/8/8/8/8 w Q - 0 1");
	REQUIRE(!b3.whiteShort);
	REQUIRE(!b3.blackShort);
	REQUIRE(b3.whiteLong);
	REQUIRE(!b3.blackLong);
	const auto b4
	    = fen("8/8/8/8/8/8/8/8 w q - 0 1");
	REQUIRE(!b4.whiteShort);
	REQUIRE(!b4.blackShort);
	REQUIRE(!b4.whiteLong);
	REQUIRE(b4.blackLong);
	const auto b5
	    = fen("8/8/8/8/8/8/8/8 w KQ - 0 1");
	REQUIRE(b5.whiteShort);
	REQUIRE(!b5.blackShort);
	REQUIRE(b5.whiteLong);
	REQUIRE(!b5.blackLong);
	const auto b6
	    = fen("8/8/8/8/8/8/8/8 w kq - 0 1");
	REQUIRE(!b6.whiteShort);
	REQUIRE(b6.blackShort);
	REQUIRE(!b6.whiteLong);
	REQUIRE(b6.blackLong);
    }
    SECTION("en passant") {
	const auto b1
	    = fen("8/8/8/8/8/8/8/8 w - - 0 1");
	REQUIRE(b1.epTarget == -1);
	const auto b2
	    = fen("8/8/8/8/8/8/8/8 b - e3 0 1");
	REQUIRE(b2.epTarget == 19);
	const auto b3
	    = fen("8/8/8/8/8/8/8/8 w - a6 0 1");
	REQUIRE(b3.epTarget == 47);
	const auto b4
	    = fen("8/8/8/8/8/8/8/8 w - g6 0 1");
	REQUIRE(b4.epTarget == 41);
    }
    SECTION("half move clock") {
	const auto b1
	    = fen("8/8/8/8/8/8/8/8 w - - 0 1");
	REQUIRE(b1.halfmoveClock == 0);
	const auto b2
	    = fen("8/8/8/8/8/8/8/8 w - - 14 1");
	REQUIRE(b2.halfmoveClock == 14);
	const auto b3
	    = fen("8/8/8/8/8/8/8/8 w - - 47 1");
	REQUIRE(b3.halfmoveClock == 47);
    }
    SECTION("empty board") {
	const auto b
	    = fen("8/8/8/8/8/8/8/8 w - - 0 1");
	REQUIRE(b.board.wPawns == 0ULL);
    }
    SECTION("FEN startpos") {
	const auto b
	    = fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
	REQUIRE(b.whiteToMove);
	REQUIRE(b.whiteShort);
	REQUIRE(b.blackShort);
	REQUIRE(b.whiteLong);
	REQUIRE(b.blackLong);
	REQUIRE(b.epTarget ==  -1);
	REQUIRE(b.halfmoveClock == 0);

	REQUIRE(b.board.wPawns == 65280ULL);
	REQUIRE(b.board.wKnights == 66ULL);
	REQUIRE(b.board.wBishops == 36ULL);
	REQUIRE(b.board.wQueens == 16ULL);
	REQUIRE(b.board.wKing == 8ULL);
	REQUIRE(b.board.bPawns == 71776119061217280ULL);
	REQUIRE(b.board.bKnights == 4755801206503243776ULL);
	REQUIRE(b.board.bBishops == 2594073385365405696ULL);
	REQUIRE(b.board.bQueens == 1152921504606846976ULL);
	REQUIRE(b.board.bKing == 576460752303423488ULL);
	REQUIRE(b.board.white == 65535ULL);
	REQUIRE(b.board.black == 18446462598732840960ULL);
	REQUIRE(b.board.occ == 18446462598732906495ULL);
    }
}
