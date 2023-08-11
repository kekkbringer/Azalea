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
    SECTION("bishop moves simple") {
	std::vector<Move> ml;

	auto gs = fen("8/8/4nk2/8/2B5/2K5/8/5n2 w - - 0 1");
	generateLegalMoves(gs, ml);
	REQUIRE(ml.size() == 14);
    }
    SECTION("pinned piece detection") {
	auto gs = fen("8/8/8/8/8/2KN1rk1/8/8 w - - 0 1");
	auto pinned = detectLaterallyPinnedPieces(gs);
	REQUIRE(pinned == 1048576ULL);

	gs = fen("3b2k1/8/8/2NNN3/rnNKN2q/2NNN3/3r4/3r4 w - - 0 1");
	pinned = detectLaterallyPinnedPieces(gs);
	REQUIRE(pinned == 135266304ULL);

	gs = fen("8/4k3/8/2n5/1B6/8/8/K7 b - - 0 1");
	pinned = detectDiagonallyPinnedPieces(gs);
	REQUIRE(pinned == 137438953472ULL);

	gs = fen("3R4/6B1/3q4/4n3/R2kb1N1/K1b5/1R6/8 b - - 0 1");
	pinned = detectLaterallyPinnedPieces(gs);
	pinned |= detectDiagonallyPinnedPieces(gs);
	REQUIRE(pinned == 17626545782784ULL);
    }
    SECTION("easy movegen with pinned pieces") {
	std::vector<Move> ml;

	auto gs = fen("8/5k2/5b2/8/3R4/2K5/8/8 w - - 0 1");
	generateLegalMoves(gs, ml);
	REQUIRE(ml.size() == 7);

	gs = fen("8/3q1k2/8/8/3R4/8/8/3K4 w - - 0 1");
	generateLegalMoves(gs, ml);
	REQUIRE(ml.size() == 10);

	gs = fen("8/4k3/4b3/8/8/2K5/4R3/8 b - - 0 1");
	generateLegalMoves(gs, ml);
	REQUIRE(ml.size() == 7);

	gs = fen("8/8/4qk2/8/2B5/1K6/8/8 w - - 0 1");
	generateLegalMoves(gs, ml);
	REQUIRE(ml.size() == 9);
    }
    SECTION("queen stuff") {
	std::vector<Move> ml;

	auto gs = fen("8/8/7k/8/2Q5/8/1K6/8 w - - 0 1");
	generateLegalMoves(gs, ml);
	REQUIRE(ml.size() == 33);

	gs = fen("8/8/7k/8/1KQ2r2/8/8/8 w - - 0 1");
	generateLegalMoves(gs, ml);
	REQUIRE(ml.size() == 10);

	gs = fen("8/8/4qk2/8/2Q5/8/K7/8 w - - 0 1");
	generateLegalMoves(gs, ml);
	REQUIRE(ml.size() == 8);
    }
    SECTION("knight stuff") {
	std::vector<Move> ml;

	auto gs = fen("k7/8/8/8/3K4/5N2/8/8 w - - 0 1");
	generateLegalMoves(gs, ml);
	REQUIRE(ml.size() == 15);

	gs = fen("8/8/8/8/8/8/4qk2/KN3r2 w - - 0 1");
	generateLegalMoves(gs, ml);
	REQUIRE(ml.size() == 0);

	gs = fen("1rN5/q6k/8/8/8/K7/8/8 w - - 0 1");
	generateLegalMoves(gs, ml);
	REQUIRE(ml.size() == 1);
    }
    SECTION("white pawns") {
	std::vector<Move> ml;

	auto gs = fen("8/8/8/8/8/7n/PPP4P/K7 w - - 0 1");
	generateLegalMoves(gs, ml);
	REQUIRE(ml.size() == 7);

	gs = fen("7k/8/8/8/3P4/8/8/K7 w - - 0 1");
	generateLegalMoves(gs, ml);
	REQUIRE(ml.size() == 4);

	gs = fen("7k/2P5/8/8/8/8/8/K7 w - - 0 1");
	generateLegalMoves(gs, ml);
	REQUIRE(ml.size() == 7);

	gs = fen("2n4k/2P5/8/8/8/8/8/K7 w - - 0 1");
	generateLegalMoves(gs, ml);
	REQUIRE(ml.size() == 3);

	// pinned
	gs = fen("8/8/8/6k1/8/8/1K1P2r1/8 w - - 0 1");
	generateLegalMoves(gs, ml);
	REQUIRE(ml.size() == 8);

	gs = fen("8/8/8/6k1/8/1K1P2r1/8/8 w - - 0 1");
	generateLegalMoves(gs, ml);
	REQUIRE(ml.size() == 8);

	gs = fen("3r4/8/8/3P2k1/8/8/3K4/8 w - - 0 1");
	generateLegalMoves(gs, ml);
	REQUIRE(ml.size() == 9);

	gs = fen("3r4/8/8/6k1/8/8/3P4/3K4 w - - 0 1");
	generateLegalMoves(gs, ml);
	REQUIRE(ml.size() == 6);

	gs = fen("8/8/8/6k1/5q2/8/3P4/2K5 w - - 0 1");
	generateLegalMoves(gs, ml);
	REQUIRE(ml.size() == 4);
    }
    SECTION("black pawns") {
	std::vector<Move> ml;

	auto gs = fen("7k/p4ppp/Q7/8/8/3K4/8/8 b - - 0 1");
	generateLegalMoves(gs, ml);
	REQUIRE(ml.size() == 7);

	gs = fen("7k/8/8/4p3/8/8/8/K7 b - - 0 1");
	generateLegalMoves(gs, ml);
	REQUIRE(ml.size() == 4);

	gs = fen("7k/8/8/K7/8/8/4p3/8 b - - 0 1");
	generateLegalMoves(gs, ml);
	REQUIRE(ml.size() == 7);

	gs = fen("7k/8/8/8/8/8/4p3/4K3 b - - 0 1");
	generateLegalMoves(gs, ml);
	REQUIRE(ml.size() == 3);

	// pinned
	gs = fen("3k4/3p4/8/8/8/8/3Q4/2K5 b - - 0 1");
	generateLegalMoves(gs, ml);
	REQUIRE(ml.size() == 6);

	gs = fen("2k5/3p4/8/8/6Q1/8/8/2K5 b - - 0 1");
	generateLegalMoves(gs, ml);
	REQUIRE(ml.size() == 4);

	gs = fen("8/1k1p2R1/8/8/8/8/8/2K5 b - - 0 1");
	generateLegalMoves(gs, ml);
	REQUIRE(ml.size() == 8);
    }
    SECTION("pawn captures") {
	std::vector<Move> ml;

	auto gs = fen("8/5k2/8/4b3/3P4/8/8/K7 w - - 0 1");
	generateLegalMoves(gs, ml);
	REQUIRE(ml.size() == 4);

	gs = fen("8/5k2/8/4b3/8/8/1P6/K7 w - - 0 1");
	generateLegalMoves(gs, ml);
	REQUIRE(ml.size() == 2);

	gs = fen("8/5k2/8/8/3r1q2/4P3/8/K7 w - - 0 1");
	generateLegalMoves(gs, ml);
	REQUIRE(ml.size() == 6);

	gs = fen("8/5k2/8/8/3q1r2/4P3/8/K7 w - - 0 1");
	generateLegalMoves(gs, ml);
	REQUIRE(ml.size() == 3);

	gs = fen("7K/8/8/5k2/8/3q1r2/4P3/8 w - - 0 1");
	generateLegalMoves(gs, ml);
	REQUIRE(ml.size() == 7);

	gs = fen("k7/8/8/8/1q6/4n1b1/3PPP2/4K3 w - - 0 1");
	generateLegalMoves(gs, ml);
	REQUIRE(ml.size() == 1);

	gs = fen("4k3/5p2/6B1/8/8/8/8/K7 b - - 0 1");
	generateLegalMoves(gs, ml);
	REQUIRE(ml.size() == 5);

	gs = fen("4k3/3ppp2/4N1B1/1Q6/8/8/8/K7 b - - 0 1");
	generateLegalMoves(gs, ml);
	REQUIRE(ml.size() == 1);

	// promos caps
	gs = fen("k5b1/5P2/8/3K4/8/8/8/8 w - - 0 1");
	generateLegalMoves(gs, ml);
	REQUIRE(ml.size() == 12);

	gs = fen("k4rb1/5P2/8/5K2/8/8/8/8 w - - 0 1");
	generateLegalMoves(gs, ml);
	REQUIRE(ml.size() == 8);

	gs = fen("8/3K4/8/8/6k1/8/2p5/8 b - - 0 1");
	generateLegalMoves(gs, ml);
	REQUIRE(ml.size() == 12);
    }
}
