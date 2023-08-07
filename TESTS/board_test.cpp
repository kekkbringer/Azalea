#include <catch2/catch.hpp>

#define CATCH_CONFIG_NOSTDOUT

#include <algorithm>

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
    SECTION("empty piece list") {
	const Board b1
	    = fen("8/8/8/8/8/8/8/8 w - - 0 1");
	REQUIRE(b1.pieceList.size() == 0);
    }
    SECTION("FEN startpos") {
	const Board b
	    = fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
	const auto pl = b.pieceList;
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
	    REQUIRE(std::find(pl.begin(), pl.end(),
		    Piece(Color::white, PieceType::pawn, i+80)) != pl.end());
	    REQUIRE(std::find(pl.begin(), pl.end(),
		    Piece(Color::black, PieceType::pawn, i+30)) != pl.end());
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
	REQUIRE(std::find(pl.begin(), pl.end(),
		    Piece(Color::white, PieceType::rook, 91)) != pl.end());
	REQUIRE(std::find(pl.begin(), pl.end(),
		    Piece(Color::white, PieceType::rook, 98)) != pl.end());
	REQUIRE(std::find(pl.begin(), pl.end(),
		    Piece(Color::black, PieceType::rook, 21)) != pl.end());
	REQUIRE(std::find(pl.begin(), pl.end(),
		    Piece(Color::black, PieceType::rook, 28)) != pl.end());

	// knights
	REQUIRE(b.mailbox[22].type == PieceType::knight);
	REQUIRE(b.mailbox[27].type == PieceType::knight);
	REQUIRE(b.mailbox[92].type == PieceType::knight);
	REQUIRE(b.mailbox[97].type == PieceType::knight);
	REQUIRE(b.mailbox[22].color == Color::black);
	REQUIRE(b.mailbox[27].color == Color::black);
	REQUIRE(b.mailbox[92].color == Color::white);
	REQUIRE(b.mailbox[97].color == Color::white);
	REQUIRE(std::find(pl.begin(), pl.end(),
		    Piece(Color::white, PieceType::knight, 92)) != pl.end());
	REQUIRE(std::find(pl.begin(), pl.end(),
		    Piece(Color::white, PieceType::knight, 97)) != pl.end());
	REQUIRE(std::find(pl.begin(), pl.end(),
		    Piece(Color::black, PieceType::knight, 22)) != pl.end());
	REQUIRE(std::find(pl.begin(), pl.end(),
		    Piece(Color::black, PieceType::knight, 27)) != pl.end());


	// bishops
	REQUIRE(b.mailbox[23].type == PieceType::bishop);
	REQUIRE(b.mailbox[26].type == PieceType::bishop);
	REQUIRE(b.mailbox[93].type == PieceType::bishop);
	REQUIRE(b.mailbox[96].type == PieceType::bishop);
	REQUIRE(b.mailbox[23].color == Color::black);
	REQUIRE(b.mailbox[26].color == Color::black);
	REQUIRE(b.mailbox[93].color == Color::white);
	REQUIRE(b.mailbox[96].color == Color::white);
	REQUIRE(std::find(pl.begin(), pl.end(),
		    Piece(Color::white, PieceType::bishop, 93)) != pl.end());
	REQUIRE(std::find(pl.begin(), pl.end(),
		    Piece(Color::white, PieceType::bishop, 96)) != pl.end());
	REQUIRE(std::find(pl.begin(), pl.end(),
		    Piece(Color::black, PieceType::bishop, 23)) != pl.end());
	REQUIRE(std::find(pl.begin(), pl.end(),
		    Piece(Color::black, PieceType::bishop, 26)) != pl.end());


	// queens
	REQUIRE(b.mailbox[24].type == PieceType::queen);
	REQUIRE(b.mailbox[94].type == PieceType::queen);
	REQUIRE(b.mailbox[24].color == Color::black);
	REQUIRE(b.mailbox[94].color == Color::white);
	REQUIRE(std::find(pl.begin(), pl.end(),
		    Piece(Color::white, PieceType::queen, 94)) != pl.end());
	REQUIRE(std::find(pl.begin(), pl.end(),
		    Piece(Color::black, PieceType::queen, 24)) != pl.end());

	// kings
	REQUIRE(b.mailbox[25].type == PieceType::king);
	REQUIRE(b.mailbox[95].type == PieceType::king);
	REQUIRE(b.mailbox[25].color == Color::black);
	REQUIRE(b.mailbox[95].color == Color::white);
	REQUIRE(b.whiteKing == Piece(Color::white, PieceType::king, 95));
	REQUIRE(b.blackKing == Piece(Color::black, PieceType::king, 25));
    }
    SECTION("FEN #2") {
	const Board b
		= fen("8/5bp1/B2k4/PP3R1p/3P2p1/P1Kn4/1Q4r1/4B3 w - - 0 1");
	const auto pl = b.pieceList;
	REQUIRE(b.whiteToMove);
	REQUIRE(!b.whiteShort);
	REQUIRE(!b.blackShort);
	REQUIRE(!b.whiteLong);
	REQUIRE(!b.blackLong);
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

	// empty squares
	REQUIRE(b.mailbox[ 21].type == PieceType::empty);
	REQUIRE(b.mailbox[ 22].type == PieceType::empty);
	REQUIRE(b.mailbox[ 23].type == PieceType::empty);
	REQUIRE(b.mailbox[ 24].type == PieceType::empty);
	REQUIRE(b.mailbox[ 25].type == PieceType::empty);
	REQUIRE(b.mailbox[ 26].type == PieceType::empty);
	REQUIRE(b.mailbox[ 27].type == PieceType::empty);
	REQUIRE(b.mailbox[ 28].type == PieceType::empty);
	REQUIRE(b.mailbox[ 31].type == PieceType::empty);
	REQUIRE(b.mailbox[ 32].type == PieceType::empty);
	REQUIRE(b.mailbox[ 33].type == PieceType::empty);
	REQUIRE(b.mailbox[ 34].type == PieceType::empty);
	REQUIRE(b.mailbox[ 35].type == PieceType::empty);
	REQUIRE(b.mailbox[ 38].type == PieceType::empty);
	REQUIRE(b.mailbox[ 42].type == PieceType::empty);
	REQUIRE(b.mailbox[ 43].type == PieceType::empty);
	REQUIRE(b.mailbox[ 45].type == PieceType::empty);
	REQUIRE(b.mailbox[ 46].type == PieceType::empty);
	REQUIRE(b.mailbox[ 47].type == PieceType::empty);
	REQUIRE(b.mailbox[ 48].type == PieceType::empty);
	REQUIRE(b.mailbox[ 53].type == PieceType::empty);
	REQUIRE(b.mailbox[ 54].type == PieceType::empty);
	REQUIRE(b.mailbox[ 55].type == PieceType::empty);
	REQUIRE(b.mailbox[ 57].type == PieceType::empty);
	REQUIRE(b.mailbox[ 61].type == PieceType::empty);
	REQUIRE(b.mailbox[ 62].type == PieceType::empty);
	REQUIRE(b.mailbox[ 63].type == PieceType::empty);
	REQUIRE(b.mailbox[ 65].type == PieceType::empty);
	REQUIRE(b.mailbox[ 66].type == PieceType::empty);
	REQUIRE(b.mailbox[ 68].type == PieceType::empty);
	REQUIRE(b.mailbox[ 72].type == PieceType::empty);
	REQUIRE(b.mailbox[ 75].type == PieceType::empty);
	REQUIRE(b.mailbox[ 76].type == PieceType::empty);
	REQUIRE(b.mailbox[ 77].type == PieceType::empty);
	REQUIRE(b.mailbox[ 78].type == PieceType::empty);
	REQUIRE(b.mailbox[ 81].type == PieceType::empty);
	REQUIRE(b.mailbox[ 83].type == PieceType::empty);
	REQUIRE(b.mailbox[ 84].type == PieceType::empty);
	REQUIRE(b.mailbox[ 85].type == PieceType::empty);
	REQUIRE(b.mailbox[ 86].type == PieceType::empty);
	REQUIRE(b.mailbox[ 88].type == PieceType::empty);
	REQUIRE(b.mailbox[ 91].type == PieceType::empty);
	REQUIRE(b.mailbox[ 92].type == PieceType::empty);
	REQUIRE(b.mailbox[ 93].type == PieceType::empty);
	REQUIRE(b.mailbox[ 94].type == PieceType::empty);
	REQUIRE(b.mailbox[ 96].type == PieceType::empty);
	REQUIRE(b.mailbox[ 97].type == PieceType::empty);
	REQUIRE(b.mailbox[ 98].type == PieceType::empty);

	// pawns
	REQUIRE(b.mailbox[51].type == PieceType::pawn);
	REQUIRE(b.mailbox[51].color == Color::white);
	REQUIRE(std::find(pl.begin(), pl.end(),
		    Piece(Color::white, PieceType::pawn, 51)) != pl.end());
	REQUIRE(b.mailbox[52].type == PieceType::pawn);
	REQUIRE(b.mailbox[52].color == Color::white);
	REQUIRE(std::find(pl.begin(), pl.end(),
		    Piece(Color::white, PieceType::pawn, 52)) != pl.end());
	REQUIRE(b.mailbox[64].type == PieceType::pawn);
	REQUIRE(b.mailbox[64].color == Color::white);
	REQUIRE(std::find(pl.begin(), pl.end(),
		    Piece(Color::white, PieceType::pawn, 64)) != pl.end());
	REQUIRE(b.mailbox[71].type == PieceType::pawn);
	REQUIRE(b.mailbox[71].color == Color::white);
	REQUIRE(std::find(pl.begin(), pl.end(),
		    Piece(Color::white, PieceType::pawn, 71)) != pl.end());

	REQUIRE(b.mailbox[37].type == PieceType::pawn);
	REQUIRE(b.mailbox[37].color == Color::black);
	REQUIRE(std::find(pl.begin(), pl.end(),
		    Piece(Color::black, PieceType::pawn, 37)) != pl.end());
	REQUIRE(b.mailbox[58].type == PieceType::pawn);
	REQUIRE(b.mailbox[58].color == Color::black);
	REQUIRE(std::find(pl.begin(), pl.end(),
		    Piece(Color::black, PieceType::pawn, 58)) != pl.end());
	REQUIRE(b.mailbox[67].type == PieceType::pawn);
	REQUIRE(b.mailbox[67].color == Color::black);
	REQUIRE(std::find(pl.begin(), pl.end(),
		    Piece(Color::black, PieceType::pawn, 67)) != pl.end());

	// knight(s)
	REQUIRE(b.mailbox[74].type == PieceType::knight);
	REQUIRE(b.mailbox[74].color == Color::black);
	REQUIRE(std::find(pl.begin(), pl.end(),
		    Piece(Color::black, PieceType::knight, 74)) != pl.end());

	// bishops
	REQUIRE(b.mailbox[41].type == PieceType::bishop);
	REQUIRE(b.mailbox[41].color == Color::white);
	REQUIRE(std::find(pl.begin(), pl.end(),
		    Piece(Color::white, PieceType::bishop, 41)) != pl.end());
	REQUIRE(b.mailbox[95].type == PieceType::bishop);
	REQUIRE(b.mailbox[95].color == Color::white);
	REQUIRE(std::find(pl.begin(), pl.end(),
		    Piece(Color::white, PieceType::bishop, 95)) != pl.end());
	REQUIRE(b.mailbox[36].type == PieceType::bishop);
	REQUIRE(b.mailbox[36].color == Color::black);
	REQUIRE(std::find(pl.begin(), pl.end(),
		    Piece(Color::black, PieceType::bishop, 36)) != pl.end());

	// rooks
	REQUIRE(b.mailbox[56].type == PieceType::rook);
	REQUIRE(b.mailbox[56].color == Color::white);
	REQUIRE(std::find(pl.begin(), pl.end(),
		    Piece(Color::white, PieceType::rook, 56)) != pl.end());
	REQUIRE(b.mailbox[87].type == PieceType::rook);
	REQUIRE(b.mailbox[87].color == Color::black);
	REQUIRE(std::find(pl.begin(), pl.end(),
		    Piece(Color::black, PieceType::rook, 87)) != pl.end());

	// queen(s)
	REQUIRE(b.mailbox[82].type == PieceType::queen);
	REQUIRE(b.mailbox[82].color == Color::white);
	REQUIRE(std::find(pl.begin(), pl.end(),
		    Piece(Color::white, PieceType::queen, 82)) != pl.end());

	// kings
	REQUIRE(b.mailbox[73].type == PieceType::king);
	REQUIRE(b.mailbox[44].type == PieceType::king);
	REQUIRE(b.mailbox[44].color == Color::black);
	REQUIRE(b.mailbox[73].color == Color::white);
	REQUIRE(b.whiteKing == Piece(Color::white, PieceType::king, 73));
	REQUIRE(b.blackKing == Piece(Color::black, PieceType::king, 44));
    }
}
