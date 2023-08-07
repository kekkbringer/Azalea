#include <catch2/catch.hpp>

#define CATCH_CONFIG_NOSTDOUT

#include "board.hpp"
#include "fen.hpp"

TEST_CASE("Testing board representation...", "[board]") {
    SECTION("side to move") {
	const Board bw
	    = fen("8/8/8/8/8/8/8/8 w - - 0 1");
	REQUIRE(bw.whiteToMove);
	const Board bb
	    = fen("8/8/8/8/8/8/8/8 b - - 0 1");
	REQUIRE(!bb.whiteToMove);
    }
    SECTION("castle rights") {
	const Board b1
	    = fen("8/8/8/8/8/8/8/8 w K - 0 1");
	REQUIRE(b1.whiteShort);
	REQUIRE(!b1.blackShort);
	REQUIRE(!b1.whiteLong);
	REQUIRE(!b1.blackLong);
	const Board b2
	    = fen("8/8/8/8/8/8/8/8 w k - 0 1");
	REQUIRE(!b2.whiteShort);
	REQUIRE(b2.blackShort);
	REQUIRE(!b2.whiteLong);
	REQUIRE(!b2.blackLong);
	const Board b3
	    = fen("8/8/8/8/8/8/8/8 w Q - 0 1");
	REQUIRE(!b3.whiteShort);
	REQUIRE(!b3.blackShort);
	REQUIRE(b3.whiteLong);
	REQUIRE(!b3.blackLong);
	const Board b4
	    = fen("8/8/8/8/8/8/8/8 w q - 0 1");
	REQUIRE(!b4.whiteShort);
	REQUIRE(!b4.blackShort);
	REQUIRE(!b4.whiteLong);
	REQUIRE(b4.blackLong);
	const Board b5
	    = fen("8/8/8/8/8/8/8/8 w KQ - 0 1");
	REQUIRE(b5.whiteShort);
	REQUIRE(!b5.blackShort);
	REQUIRE(b5.whiteLong);
	REQUIRE(!b5.blackLong);
	const Board b6
	    = fen("8/8/8/8/8/8/8/8 w kq - 0 1");
	REQUIRE(!b6.whiteShort);
	REQUIRE(b6.blackShort);
	REQUIRE(!b6.whiteLong);
	REQUIRE(b6.blackLong);
    }
    SECTION("en passant") {
	const Board b1
	    = fen("8/8/8/8/8/8/8/8 w - - 0 1");
	REQUIRE(b1.enPassantTarget == 0);
	const Board b2
	    = fen("8/8/8/8/8/8/8/8 b - e3 0 1");
	REQUIRE(b2.enPassantTarget == 75);
	const Board b3
	    = fen("8/8/8/8/8/8/8/8 w - a6 0 1");
	REQUIRE(b3.enPassantTarget == 41);
	const Board b4
	    = fen("8/8/8/8/8/8/8/8 w - g6 0 1");
	REQUIRE(b4.enPassantTarget == 47);
    }
    SECTION("half move clock") {
	const Board b1
	    = fen("8/8/8/8/8/8/8/8 w - - 0 1");
	REQUIRE(b1.halfmoveClock == 0);
	const Board b2
	    = fen("8/8/8/8/8/8/8/8 w - - 14 1");
	REQUIRE(b2.halfmoveClock == 14);
	const Board b3
	    = fen("8/8/8/8/8/8/8/8 w - - 47 1");
	REQUIRE(b3.halfmoveClock == 47);
    }
    SECTION("FEN startpos") {
	const Board b
	    = fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
	REQUIRE(b.whiteToMove);
	REQUIRE(b.whiteShort);
	REQUIRE(b.blackShort);
	REQUIRE(b.whiteLong);
	REQUIRE(b.blackLong);
	REQUIRE(b.enPassantTarget ==  0);
	REQUIRE(b.halfmoveClock == 0);

	// out of Board
	for (int i=0; i<10; i++) {
	    REQUIRE(b.mailbox[i].type       == PieceType::outOfBoard);
	    REQUIRE(b.mailbox[i + 10].type  == PieceType::outOfBoard);
	    REQUIRE(b.mailbox[i + 100].type == PieceType::outOfBoard);
	    REQUIRE(b.mailbox[i + 110].type == PieceType::outOfBoard);
	}
	for (int i=2; i<10; i++) {
	    REQUIRE(b.mailbox[i*10].type   == PieceType::outOfBoard);
	    REQUIRE(b.mailbox[i*10+9].type == PieceType::outOfBoard);
	}

	// empty squares and pawns
	for (int i=1; i<9; i++) {
	    REQUIRE(b.mailbox[i + 40].type == PieceType::empty);
	    REQUIRE(b.mailbox[i + 50].type == PieceType::empty);
	    REQUIRE(b.mailbox[i + 60].type == PieceType::empty);
	    REQUIRE(b.mailbox[i + 70].type == PieceType::empty);

	    REQUIRE(b.mailbox[i + 30].type == PieceType::pawn);
	    REQUIRE(b.mailbox[i + 80].type == PieceType::pawn);
	    REQUIRE(b.mailbox[i + 30].color == Color::black);
	    REQUIRE(b.mailbox[i + 80].color == Color::white);
	}

	// rooks
	REQUIRE(b.mailbox[21].type == PieceType::rook);
	REQUIRE(b.mailbox[28].type == PieceType::rook);
	REQUIRE(b.mailbox[91].type == PieceType::rook);
	REQUIRE(b.mailbox[98].type == PieceType::rook);
	REQUIRE(b.mailbox[21].color == Color::black);
	REQUIRE(b.mailbox[28].color == Color::black);
	REQUIRE(b.mailbox[91].color == Color::white);
	REQUIRE(b.mailbox[98].color == Color::white);

	// knights
	REQUIRE(b.mailbox[22].type == PieceType::knight);
	REQUIRE(b.mailbox[27].type == PieceType::knight);
	REQUIRE(b.mailbox[92].type == PieceType::knight);
	REQUIRE(b.mailbox[97].type == PieceType::knight);
	REQUIRE(b.mailbox[22].color == Color::black);
	REQUIRE(b.mailbox[27].color == Color::black);
	REQUIRE(b.mailbox[92].color == Color::white);
	REQUIRE(b.mailbox[97].color == Color::white);


	// bishops
	REQUIRE(b.mailbox[23].type == PieceType::bishop);
	REQUIRE(b.mailbox[26].type == PieceType::bishop);
	REQUIRE(b.mailbox[93].type == PieceType::bishop);
	REQUIRE(b.mailbox[96].type == PieceType::bishop);
	REQUIRE(b.mailbox[23].color == Color::black);
	REQUIRE(b.mailbox[26].color == Color::black);
	REQUIRE(b.mailbox[93].color == Color::white);
	REQUIRE(b.mailbox[96].color == Color::white);


	// queens
	REQUIRE(b.mailbox[24].type == PieceType::queen);
	REQUIRE(b.mailbox[94].type == PieceType::queen);
	REQUIRE(b.mailbox[24].color == Color::black);
	REQUIRE(b.mailbox[94].color == Color::white);

	// kings
	REQUIRE(b.mailbox[25].type == PieceType::king);
	REQUIRE(b.mailbox[95].type == PieceType::king);
	REQUIRE(b.mailbox[25].color == Color::black);
	REQUIRE(b.mailbox[95].color == Color::white);
    }
}
