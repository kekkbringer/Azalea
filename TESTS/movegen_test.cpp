#include <catch2/catch.hpp>

#define CATCH_CONFIG_NOSTDOUT

#include <vector>

#include "board.hpp"
#include "move.hpp"
#include "movegenerator.hpp"
#include "fen.hpp"
#include "util.hpp"

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
	const Board b = fen("1r6/8/8/4k3/8/8/8/K7 w - - 0 1");
	generateLegalMoves(b, movelist);
	REQUIRE(movelist.size()==1);

	const Board b2 = fen("8/8/8/8/8/2k5/8/K7 w - - 0 1");
	generateLegalMoves(b2, movelist);
	REQUIRE(movelist.size()==2);

	const Board b3 = fen("8/8/8/4k3/1n6/8/8/K7 w - - 0 1");
	generateLegalMoves(b3, movelist);
	REQUIRE(movelist.size()==2);

	const Board b4 = fen("3rk3/8/8/8/8/3K4/8/8 w - - 0 1");
	generateLegalMoves(b4, movelist);
	REQUIRE(movelist.size()==6);

	const Board b5 = fen("3rk3/8/8/5n2/8/3K4/8/8 w - - 0 1");
	generateLegalMoves(b5, movelist);
	REQUIRE(movelist.size()==5);

	const Board b6 = fen("b2rk3/8/8/5n2/8/3K4/8/8 w - - 0 1");
	generateLegalMoves(b6, movelist);
	REQUIRE(movelist.size()==4);

	const Board b7 = fen("b2rk3/8/8/5n2/8/3K4/8/2q5 w - - 0 1");
	generateLegalMoves(b7, movelist);
	REQUIRE(movelist.size()==1);

	const Board b8 = fen("b2r4/8/8/8/8/3K4/8/2q2k2 w - - 0 1");
	generateLegalMoves(b8, movelist);
	REQUIRE(movelist.size()==0);
    }

    SECTION("checker detection") {
	const Board b = fen("8/3r1k2/8/8/8/3K4/8/8 w - - 0 1");
	const auto checker = detectChecker(b);
	REQUIRE(checker.size() == 1);
	REQUIRE(checker[0].type == PieceType::rook);
	REQUIRE(checker[0].color == Color::black);
	REQUIRE(checker[0].location == 34);

	const Board b2 = fen("8/1r3k2/5R2/8/8/3K4/8/8 b - - 0 1");
	const auto checker2 = detectChecker(b2);
	REQUIRE(checker2.size() == 1);
	REQUIRE(checker2[0].type == PieceType::rook);
	REQUIRE(checker2[0].color == Color::white);
	REQUIRE(checker2[0].location == 46);

	const Board b3 = fen("8/4k3/8/8/8/2K3b1/8/8 w - - 0 1");
	const auto checker3 = detectChecker(b3);
	REQUIRE(checker3.size() == 0);

	const Board b4 = fen("8/4k3/5b2/8/8/2K5/8/8 w - - 0 1");
	const auto checker4 = detectChecker(b4);
	REQUIRE(checker4.size() == 1);
	REQUIRE(checker4[0].type == PieceType::bishop);
	REQUIRE(checker4[0].color == Color::black);
	REQUIRE(checker4[0].location == 46);

	const Board b5 = fen("8/4k3/8/q7/8/2K5/8/8 w - - 0 1");
	const auto checker5 = detectChecker(b5);
	REQUIRE(checker5.size() == 1);
	REQUIRE(checker5[0].type == PieceType::queen);
	REQUIRE(checker5[0].color == Color::black);
	REQUIRE(checker5[0].location == 51);

	const Board b6 = fen("8/4N3/2k5/8/8/5K2/8/8 b - - 0 1");
	const auto checker6 = detectChecker(b6);
	REQUIRE(checker6.size() == 1);
	REQUIRE(checker6[0].type == PieceType::knight);
	REQUIRE(checker6[0].color == Color::white);
	REQUIRE(checker6[0].location == 35);

	const Board b7 = fen("8/8/2k5/8/8/5K2/8/4n3 w - - 0 1");
	const auto checker7 = detectChecker(b7);
	REQUIRE(checker7.size() == 1);
	REQUIRE(checker7[0].type == PieceType::knight);
	REQUIRE(checker7[0].color == Color::black);
	REQUIRE(checker7[0].location == 95);

	const Board b8 = fen("8/8/8/3k4/3p4/4K3/8/8 w - - 0 1");
	const auto checker8 = detectChecker(b8);
	REQUIRE(checker8.size() == 1);
	REQUIRE(checker8[0].type == PieceType::pawn);
	REQUIRE(checker8[0].color == Color::black);
	REQUIRE(checker8[0].location == 64);

	const Board b9 = fen("8/8/8/3k4/3p4/2K5/8/8 w - - 0 1");
	const auto checker9 = detectChecker(b9);
	REQUIRE(checker9.size() == 1);
	REQUIRE(checker9[0].type == PieceType::pawn);
	REQUIRE(checker9[0].color == Color::black);
	REQUIRE(checker9[0].location == 64);

	const Board b10 = fen("8/8/8/3k4/2P5/2K5/8/8 b - - 0 1");
	const auto checker10 = detectChecker(b10);
	REQUIRE(checker10.size() == 1);
	REQUIRE(checker10[0].type == PieceType::pawn);
	REQUIRE(checker10[0].color == Color::white);
	REQUIRE(checker10[0].location == 63);

	const Board b11 = fen("8/8/8/2K5/2P5/3k4/8/8 b - - 0 1");
	const auto checker11 = detectChecker(b11);
	REQUIRE(checker11.size() == 0);

	const Board b12 = fen("8/8/8/1K6/2p5/3k4/8/8 w - - 0 1");
	const auto checker12 = detectChecker(b12);
	REQUIRE(checker12.size() == 0);

	const Board b13 = fen("3r4/8/4k3/4n3/8/3K4/8/8 w - - 0 1");
	const auto checker13 = detectChecker(b13);
	REQUIRE(checker13.size() == 2);
	REQUIRE(std::find(checker13.begin(), checker13.end(),
	    Piece(Color::black, PieceType::rook, 24)) != checker13.end());
	REQUIRE(std::find(checker13.begin(), checker13.end(),
	    Piece(Color::black, PieceType::knight, 55)) != checker13.end());
    }

    SECTION("pinned piece detection") {
	const Board b = fen("3r2k1/8/8/8/3N4/3K4/8/8 w - - 0 1");
	const auto pinned = detectPinnedPieces(b);
	REQUIRE(pinned.size() == 1);
	REQUIRE(pinned[0] == 64);

	const Board b2 = fen("8/8/1Q2rk2/2K5/8/8/8/8 b - - 0 1");
	const auto pinned2 = detectPinnedPieces(b2);
	REQUIRE(pinned2.size() == 1);
	REQUIRE(pinned2[0] == 45);
    }
}
