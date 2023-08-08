#include <catch2/catch.hpp>

#define CATCH_CONFIG_NOSTDOUT

#include "board.hpp"
#include "util.hpp"
#include "fen.hpp"

TEST_CASE("testing isAttackedNoKing function...", "[attNoKing]") {
    SECTION("rook") {
	const Board b = fen("8/8/8/4r3/8/8/8/8 w - - 0 1");
	REQUIRE(isAttackedNoKing(b, 65));
	REQUIRE(isAttackedNoKing(b, 75));
	REQUIRE(isAttackedNoKing(b, 85));
	REQUIRE(isAttackedNoKing(b, 95));
	REQUIRE(isAttackedNoKing(b, 25));
	REQUIRE(isAttackedNoKing(b, 35));
	REQUIRE(isAttackedNoKing(b, 45));
	REQUIRE(isAttackedNoKing(b, 51));
	REQUIRE(isAttackedNoKing(b, 52));
	REQUIRE(isAttackedNoKing(b, 53));
	REQUIRE(isAttackedNoKing(b, 54));
	REQUIRE(isAttackedNoKing(b, 56));
	REQUIRE(isAttackedNoKing(b, 57));
	REQUIRE(isAttackedNoKing(b, 58));

	REQUIRE(!isAttackedNoKing(b, 91));
	REQUIRE(!isAttackedNoKing(b, 93));
	REQUIRE(!isAttackedNoKing(b, 87));
	REQUIRE(!isAttackedNoKing(b, 33));

	const Board b2 = fen("8/8/8/4R3/8/8/8/8 b - - 0 1");
	REQUIRE(isAttackedNoKing(b2, 65));
	REQUIRE(isAttackedNoKing(b2, 75));
	REQUIRE(isAttackedNoKing(b2, 85));
	REQUIRE(isAttackedNoKing(b2, 95));
	REQUIRE(isAttackedNoKing(b2, 25));
	REQUIRE(isAttackedNoKing(b2, 35));
	REQUIRE(isAttackedNoKing(b2, 45));
	REQUIRE(isAttackedNoKing(b2, 51));
	REQUIRE(isAttackedNoKing(b2, 52));
	REQUIRE(isAttackedNoKing(b2, 53));
	REQUIRE(isAttackedNoKing(b2, 54));
	REQUIRE(isAttackedNoKing(b2, 56));
	REQUIRE(isAttackedNoKing(b2, 57));
	REQUIRE(isAttackedNoKing(b2, 58));

	REQUIRE(!isAttackedNoKing(b2, 91));
	REQUIRE(!isAttackedNoKing(b2, 93));
	REQUIRE(!isAttackedNoKing(b2, 87));
	REQUIRE(!isAttackedNoKing(b2, 33));
    }

    SECTION("bishop") {
	const Board b = fen("8/8/8/4b3/8/8/8/8 w - - 0 1");
	REQUIRE(isAttackedNoKing(b, 44));
	REQUIRE(isAttackedNoKing(b, 33));
	REQUIRE(isAttackedNoKing(b, 22));
	REQUIRE(isAttackedNoKing(b, 66));
	REQUIRE(isAttackedNoKing(b, 77));
	REQUIRE(isAttackedNoKing(b, 88));
	REQUIRE(isAttackedNoKing(b, 46));
	REQUIRE(isAttackedNoKing(b, 37));
	REQUIRE(isAttackedNoKing(b, 28));
	REQUIRE(isAttackedNoKing(b, 64));
	REQUIRE(isAttackedNoKing(b, 73));
	REQUIRE(isAttackedNoKing(b, 82));
	REQUIRE(isAttackedNoKing(b, 91));

	REQUIRE(!isAttackedNoKing(b, 51));
	REQUIRE(!isAttackedNoKing(b, 31));
	REQUIRE(!isAttackedNoKing(b, 54));
	REQUIRE(!isAttackedNoKing(b, 96));

	const Board b2 = fen("8/8/8/4B3/8/8/8/8 b - - 0 1");
	REQUIRE(isAttackedNoKing(b2, 44));
	REQUIRE(isAttackedNoKing(b2, 33));
	REQUIRE(isAttackedNoKing(b2, 22));
	REQUIRE(isAttackedNoKing(b2, 66));
	REQUIRE(isAttackedNoKing(b2, 77));
	REQUIRE(isAttackedNoKing(b2, 88));
	REQUIRE(isAttackedNoKing(b2, 46));
	REQUIRE(isAttackedNoKing(b2, 37));
	REQUIRE(isAttackedNoKing(b2, 28));
	REQUIRE(isAttackedNoKing(b2, 64));
	REQUIRE(isAttackedNoKing(b2, 73));
	REQUIRE(isAttackedNoKing(b2, 82));
	REQUIRE(isAttackedNoKing(b2, 91));

	REQUIRE(!isAttackedNoKing(b2, 51));
	REQUIRE(!isAttackedNoKing(b2, 31));
	REQUIRE(!isAttackedNoKing(b2, 54));
	REQUIRE(!isAttackedNoKing(b2, 96));
    }

    SECTION("queen") {
	const Board b = fen("8/1q6/8/8/8/8/8/8 w - - 0 1");
	REQUIRE(isAttackedNoKing(b, 72));
	REQUIRE(isAttackedNoKing(b, 35));
	REQUIRE(isAttackedNoKing(b, 54));
	REQUIRE(isAttackedNoKing(b, 21));
	REQUIRE(!isAttackedNoKing(b, 32));
	REQUIRE(!isAttackedNoKing(b, 93));
    }

    SECTION("blocker") {
	const Board b = fen("8/8/8/8/8/8/8/4N2r w - - 0 1");
	REQUIRE(isAttackedNoKing(b, 95));
	REQUIRE(isAttackedNoKing(b, 96));
	REQUIRE(!isAttackedNoKing(b, 91));
	REQUIRE(!isAttackedNoKing(b, 92));

	const Board b2 = fen("8/8/2B5/3p4/8/8/8/8 b - - 0 1");
	REQUIRE(isAttackedNoKing(b2, 54));
	REQUIRE(isAttackedNoKing(b2, 21));
	REQUIRE(!isAttackedNoKing(b2, 98));
	REQUIRE(!isAttackedNoKing(b2, 65));
    }

    SECTION("knight") {
	const Board b = fen("8/8/8/4n3/8/8/8/8 w - - 0 1");
	REQUIRE(isAttackedNoKing(b, 76));
	REQUIRE(!isAttackedNoKing(b, 25));

	const Board b2 = fen("8/8/8/4N3/8/8/8/8 w - - 0 1");
	REQUIRE(!isAttackedNoKing(b2, 76));
    }

    SECTION("king") {
	const Board b = fen("8/8/8/8/8/8/8/K7 b - - 0 1");
	REQUIRE(isAttackedNoKing(b, 81));
	REQUIRE(isAttackedNoKing(b, 82));
	REQUIRE(isAttackedNoKing(b, 92));
	REQUIRE(!isAttackedNoKing(b, 91));
	REQUIRE(!isAttackedNoKing(b, 27));
    }

    SECTION("pawn") {
	const Board b = fen("8/8/8/8/3p4/8/8/8 w - - 0 1");
	REQUIRE(isAttackedNoKing(b, 73));
	REQUIRE(isAttackedNoKing(b, 75));
	REQUIRE(!isAttackedNoKing(b, 53));
	REQUIRE(!isAttackedNoKing(b, 55));

	const Board b2 = fen("8/8/8/8/3P4/8/8/8 b - - 0 1");
	REQUIRE(!isAttackedNoKing(b2, 73));
	REQUIRE(!isAttackedNoKing(b2, 75));
	REQUIRE(isAttackedNoKing(b2, 53));
	REQUIRE(isAttackedNoKing(b2, 55));
    }
}
