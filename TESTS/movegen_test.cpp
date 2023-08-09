#include <catch2/catch.hpp>

#define CATCH_CONFIG_NOSTDOUT

#include "board.hpp"
#include "movegenerator.hpp"
#include "fen.hpp"

TEST_CASE("Testing movegen...", "[board]") {
    SECTION("rook attacks") {
	auto gs
	    = fen("8/8/8/4R3/8/8/8/8 w - - 0 1");
	auto atks = rookAttacks(gs, 35);
	REQUIRE(atks == 578722409201797128ULL);

	gs = fen("8/4n3/8/2K1R1q1/8/4B3/8/8 w - - 0 1");
	atks = rookAttacks(gs, 35);
	REQUIRE(atks == 2260827969683456ULL);

	gs = fen("8/1B6/8/8/8/8/8/8 w - - 0 1");
	atks = bishopAttacks(gs, 54);
	REQUIRE(atks == 11529391036782871041ULL);

	gs = fen("2n5/1B6/8/8/4K3/8/8/8 w - - 0 1");
	atks = bishopAttacks(gs, 54);
	REQUIRE(atks == 11529391036782608384ULL);
    }
}
