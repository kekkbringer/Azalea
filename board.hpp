#pragma once

#include <vector>

/******************************************************************************
 * This file contains the board representation as well as some auxiliary
 * definitions
 *****************************************************************************/


// enum class for player color
enum class Color {white, black};

// enum class for piece type
enum class PieceType {pawn, knight, bishop, rook, queen, king,
			empty, outOfBoard};

// enum class for pinning state
enum class PinState {unpinned,
			vertical, horizontal,
			diagonal, antidiagonal};

/******************************************************************************
 * PieceInfo class
 *
 * Consists of the piece color and the piece type. There are two special
 * pieces, namely the 'empty' piece, used for empty squares on the board,
 * and the 'outOfBoard' piece, used fpr out-of-board detection for the
 * 10x12 mailbox.
 */
class PieceInfo {
public:
    Color color;
    PieceType type;

    PieceInfo()
	: color(Color::white)
	, type(PieceType::empty)
	{}

    PieceInfo(PieceType type)
	: color(Color::white)
	, type(type)
	{}

    PieceInfo(Color color, PieceType type)
	: color(color)
	, type(type)
	{}
};

/******************************************************************************
 * Piece class
 *
 * Similar to PieceInfo but contains some additional information:
 * 	- its location on the board
 * DELETED	- has it moved yet?
 */
class Piece {
public:
    Color color;
    PieceType type;
    int location; // index on the 10x12 board
    PinState pinState;

    Piece()
	: color(Color::white)
	, type(PieceType::empty)
	, location(0)
	{}

    Piece(Color color, PieceType type, int location)
	: color(color)
	, type(type)
	, location(location)
	{}

    bool operator==(const Piece& p) const {
	return color==p.color and type==p.type and location==p.location;
    }

    bool operator!=(const Piece& p) const {
	return !(color==p.color and type==p.type and location==p.location);
    }
 };
 
/******************************************************************************
 * Board class
 *
 * Consists of a 10x12 mailbox containing objects of 'PieceInfo' and a redun-
 * dant vector of 'Piece's. Additionally keeps track of which side is to move,
 * castling rights, the (possible) en passant target square and the halfmove
 * clock.
 *
 *         a   b   c   d   e   f   g   h
 *
 *   0 |   1   2   3   4   5   6   7   8 |   9
 *  10 |  11  12  13  14  15  16  17  18 |  19
 *  ---|---------------------------------|----
 *  20 |  21  22  23  24  25  26  27  28 |  29    8
 *  30 |  31  32  33  34  35  36  37  38 |  39    7
 *  40 |  41  42  43  44  45  46  47  48 |  49    6
 *  50 |  51  52  53  54  55  56  57  58 |  59    5
 *  60 |  61  62  63  64  65  66  67  68 |  69    4
 *  70 |  71  72  73  74  75  76  77  78 |  79    3
 *  80 |  81  82  83  84  85  86  87  88 |  89    2
 *  90 |  91  92  93  94  95  96  97  98 |  99    1
 *  ---|---------------------------------|----
 * 100 | 101 102 103 104 105 106 107 108 | 109
 * 110 | 111 112 113 114 115 116 117 118 | 119
 */
class Board {
public:
    // piece arrangement
    PieceInfo mailbox[120];
    std::vector<Piece> pieceList;

    // kings
    Piece whiteKing;
    Piece blackKing;

    // side to move
    bool whiteToMove;

    // castling rights
    bool whiteShort;
    bool whiteLong;
    bool blackShort;
    bool blackLong;

    // en passant target
    int enPassantTarget;

    // halfmove clock
    int halfmoveClock;

    Board()
	: whiteToMove(true)
	, whiteShort(false)
	, whiteLong(false)
	, blackShort(false)
	, blackLong(false)
	, enPassantTarget(0)
	, halfmoveClock(0)
	{
	    // set every square to empty
	    for (int i=0; i<120; i++) mailbox[i] = PieceInfo();

	    // set out of board squares
	    for (int i=0; i<10; i++)
		mailbox[i] = PieceInfo(PieceType::outOfBoard);
	    for (int i=10; i<20; i++)
		mailbox[i] = PieceInfo(PieceType::outOfBoard);
	    for (int i=100; i<110; i++)
		mailbox[i] = PieceInfo(PieceType::outOfBoard);
	    for (int i=110; i<120; i++)
		mailbox[i] = PieceInfo(PieceType::outOfBoard);
	    for (int i=2; i<10; i++) {
		mailbox[i*10]   = PieceInfo(PieceType::outOfBoard);
		mailbox[i*10+9] = PieceInfo(PieceType::outOfBoard);
	    }
	}
};
