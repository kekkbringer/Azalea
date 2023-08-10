#include <catch2/catch.hpp>

#define CATCH_CONFIG_NOSTDOUT

#include <vector>
#include <iostream> // to be deleted

#include "board.hpp"
#include "movegenerator.hpp"
#include "fen.hpp"
#include "util.hpp"

TEST_CASE("Testing movegen...", "[board]") {
    SECTION("slider attacks") {
	auto gs
	    = fen("8/8/8/4R3/8/8/8/8 w - - 0 1");
	auto atks = rookAttacks(gs.board.occ, 35);
	REQUIRE(atks == 578722409201797128ULL);

	gs = fen("8/4n3/8/2K1R1q1/8/4B3/8/8 w - - 0 1");
	atks = rookAttacks(gs.board.occ, 35);
	REQUIRE(atks == 2260827969683456ULL);

	gs = fen("8/1B6/8/8/8/8/8/8 w - - 0 1");
	atks = bishopAttacks(gs.board.occ, 54);
	REQUIRE(atks == 11529391036782871041ULL);

	gs = fen("2n5/1B6/8/8/4K3/8/8/8 w - - 0 1");
	atks = bishopAttacks(gs.board.occ, 54);
	REQUIRE(atks == 11529391036782608384ULL);

	gs = fen("8/8/8/8/8/8/8/7R w - - 0 1");
	atks = rookAttacks(gs.board.occ, 0);
	REQUIRE(atks == 72340172838076926ULL);

	gs = fen("8/8/8/8/7K/8/8/7R w - - 0 1");
	atks = rookAttacks(gs.board.occ, 0);
	REQUIRE(atks == 16843262ULL);

	gs = fen("8/8/8/8/8/8/7K/7R w - - 0 1");
	atks = rookAttacks(gs.board.occ, 0);
	REQUIRE(atks == 510ULL);

	gs = fen("8/8/8/8/8/8/7K/6NR w - - 0 1");
	atks = rookAttacks(gs.board.occ, 0);
	REQUIRE(atks == 258ULL);

	gs = fen("8/8/8/8/8/8/K7/R3N3 w - - 0 1");
	atks = rookAttacks(gs.board.occ, 7);
	REQUIRE(atks == 32888ULL);

	gs = fen("8/8/8/8/8/8/K7/RN6 w - - 0 1");
	atks = rookAttacks(gs.board.occ, 7);
	REQUIRE(atks == 32832ULL);

	gs = fen("8/8/3k4/8/8/8/K7/RR6 w - - 0 1");
	atks = rookAttacks(gs.board.occ, 7);
	REQUIRE(atks == 32832ULL);

	gs = fen("8/8/3K4/8/8/8/k7/rr6 b - - 0 1");
	atks = rookAttacks(gs.board.occ, 7);
	REQUIRE(atks == 32832ULL);

	gs = fen("8/8/3K4/8/8/8/kB6/rr6 b - - 0 1");
	atks = rookAttacks(gs.board.occ, 7);
	REQUIRE(atks == 32832ULL);

	gs = fen("8/8/3K4/8/N7/8/rB6/rk6 b - - 0 1");
	atks = rookAttacks(gs.board.occ, 7);
	REQUIRE(atks == 32832ULL);

	gs = fen("8/8/3K4/8/8/8/kB6/rr1N4 b - - 0 1");
	atks = rookAttacks(gs.board.occ, 7);
	REQUIRE(atks == 32832ULL);

	gs = fen("8/8/8/8/3B4/4r3/8/8 w - - 0 1"); // se
	atks = bishopAttacks(gs.board.occ, 28);
	REQUIRE(atks == 108724279602331776ULL);

	gs = fen("8/8/8/8/3B4/2r5/8/8 w - - 0 1"); // sw
	atks = bishopAttacks(gs.board.occ, 28);
	REQUIRE(atks == 108724279602316290ULL);

	gs = fen("8/8/8/2r5/3B4/8/8/8 w - - 0 1"); // nw
	atks = bishopAttacks(gs.board.occ, 28);
	REQUIRE(atks == 72625113839191170ULL);

	gs = fen("8/8/8/4r3/3B4/8/8/8 w - - 0 1"); // ne
	atks = bishopAttacks(gs.board.occ, 28);
	REQUIRE(atks == 36099337564472450ULL);
    }
    SECTION("pawn attacks") {
	auto gs
	    = fen("8/8/7P/6P1/5P2/1PP1P3/P2P4/8 w - - 0 1");
	auto atks = pawnAttacks(gs.whiteToMove, gs.board.wPawns);
	REQUIRE(atks == 568494561689600ULL);

	gs = fen("8/8/8/3pp3/8/8/1p6/8 b - - 0 1");
	atks = pawnAttacks(gs.whiteToMove, gs.board.bPawns);
	REQUIRE(atks == 1006633120ULL);
    }
    SECTION("king danger") {
	auto gs
	    = fen("8/8/1k6/8/5n2/8/8/K7 w - - 0 1");
	auto kd = generateKingDanger(gs);
	REQUIRE(kd == 63238346848143872ULL);

	gs = fen("8/8/1K6/8/5N2/2N5/8/k7 b - - 0 1");
	kd = generateKingDanger(gs);
	REQUIRE(kd == 63238349129878096ULL);

	gs = fen("k5b1/5P2/8/5q2/2r1n3/5p2/2R3p1/K7 w - - 0 1");
	kd = generateKingDanger(gs);
	REQUIRE(kd == 6986559648420019717ULL);
    }
    SECTION("king moves, no castle") {
	std::vector<Move> ml;
	auto gs = fen("1q1kb3/8/2n5/8/4r3/8/8/K7 w - - 0 1");
	generateLegalMoves(gs, ml);
	REQUIRE(ml.size() == 1);

	gs = fen("1q1kb3/8/2n5/8/3br3/8/8/K7 w - - 0 1");
	generateLegalMoves(gs, ml);
	REQUIRE(ml.size() == 1);

	gs = fen("3R4/4N3/5K2/6B1/3k4/2Q5/8/8 b - - 0 1");
	generateLegalMoves(gs, ml);
	REQUIRE(ml.size() == 2);
    }
    SECTION("rook moves simple") {
	std::vector<Move> ml;
	auto gs = fen("8/8/3k4/8/8/2R5/1K6/8 w - - 0 1");
	generateLegalMoves(gs, ml);
	REQUIRE(ml.size() == 21);
	
	gs = fen("8/8/1r1k4/8/8/2R5/1K6/8 w - - 0 1");
	generateLegalMoves(gs, ml);
	REQUIRE(ml.size() == 6);

	gs = fen("8/8/3k4/5R2/8/2R5/1K6/8 w - - 0 1");
	generateLegalMoves(gs, ml);
	REQUIRE(ml.size() == 35);

	gs = fen("8/8/3K4/8/8/8/kB6/rr6 b - - 0 1");
	generateLegalMoves(gs, ml);
	REQUIRE(ml.size() == 9);

	gs = fen("8/8/3K4/8/8/8/kB6/rr1N4 b - - 0 1");
	generateLegalMoves(gs, ml);
	REQUIRE(ml.size() == 4);

	gs = fen("8/8/4k3/8/2K4r/4R3/4b3/8 w - - 0 1");
	generateLegalMoves(gs, ml);
	REQUIRE(ml.size() == 3);
    }
}
