#include "util.hpp"

#include <string>

#include "board.hpp"

/******************************************************************************
 * Function that converts a square in algebraic notation, e.g. 'e4', to its
 * corresponding index on the 10x12 board.
 */
size_t toIndex(std::string s) {
    size_t file = 0;
    size_t rank = 10;

    const char f = s[0];
    const char r = s[1];

    switch (f) {
	case 'a':
	    file += 1;
	    break;
	case 'b':
	    file += 2;
	    break;
	case 'c':
	    file += 3;
	    break;
	case 'd':
	    file += 4;
	    break;
	case 'e':
	    file += 5;
	    break;
	case 'f':
	    file += 6;
	    break;
	case 'g':
	    file += 7;
	    break;
	case 'h':
	    file += 8;
	    break;
    }

    switch (r) {
	case '1':
	    rank -= 1;
	    break;
	case '2':
	    rank -= 2;
	    break;
	case '3':
	    rank -= 3;
	    break;
	case '4':
	    rank -= 4;
	    break;
	case '5':
	    rank -= 5;
	    break;
	case '6':
	    rank -= 6;
	    break;
	case '7':
	    rank -= 7;
	    break;
	case '8':
	    rank -= 8;
	    break;
    }

    return 10*rank + file;
}

/******************************************************************************
 * Function that checks if a give square is attacked by the enemy. Own King is
 * not a blocker, this is for movegen purposes.
 */
bool isAttackedNoKing(const Board& b, const int index) {
    const Color ownColor = (b.whiteToMove ? Color::white : Color::black);
    const Color enemyColor = (b.whiteToMove ? Color::black : Color::white);

    const int ownKingLocation = (b.whiteToMove ? b.whiteKing.location
					    : b.blackKing.location);
    const int enemyKingLocation = (b.whiteToMove ? b.blackKing.location
					    : b.whiteKing.location);

    ///////////////////////////////////////////////////////
    //                   Rook and Queen                  //
    ///////////////////////////////////////////////////////
    // north (+10)
    for (int i=1; i<8; i++) {
	const auto sq = b.mailbox[index + 10*i];
	if (index + 10*i == ownKingLocation) continue;
	if (sq.type == PieceType::outOfBoard) {
	    break;
	} else if (sq.type == PieceType::empty) {
	    continue;
	} else if (sq.color == ownColor) {
	    break;
	} else if (sq.color == enemyColor) {
	    if (sq.type == PieceType::rook) return true;
	    if (sq.type == PieceType::queen) return true;
	}
    }
    // south (-10)
    for (int i=1; i<8; i++) {
	const auto sq = b.mailbox[index - 10*i];
	if (index - 10*i == ownKingLocation) continue;
	if (sq.type == PieceType::outOfBoard) {
	    break;
	} else if (sq.type == PieceType::empty) {
	    continue;
	} else if (sq.color == ownColor) {
	    break;
	} else if (sq.color == enemyColor) {
	    if (sq.type == PieceType::rook) return true;
	    if (sq.type == PieceType::queen) return true;
	}
    }
    // east (+1)
    for (int i=1; i<8; i++) {
	const auto sq = b.mailbox[index + i];
	if (index + i == ownKingLocation) continue;
	if (sq.type == PieceType::outOfBoard) {
	    break;
	} else if (sq.type == PieceType::empty) {
	    continue;
	} else if (sq.color == ownColor) {
	    break;
	} else if (sq.color == enemyColor) {
	    if (sq.type == PieceType::rook) return true;
	    if (sq.type == PieceType::queen) return true;
	}
    }
    // west (-1)
    for (int i=1; i<8; i++) {
	const auto sq = b.mailbox[index - i];
	if (index - i == ownKingLocation) continue;
	if (sq.type == PieceType::outOfBoard) {
	    break;
	} else if (sq.type == PieceType::empty) {
	    continue;
	} else if (sq.color == ownColor) {
	    break;
	} else if (sq.color == enemyColor) {
	    if (sq.type == PieceType::rook) return true;
	    if (sq.type == PieceType::queen) return true;
	}
    }

    ///////////////////////////////////////////////////////
    //                 Bishop and Queen                  //
    ///////////////////////////////////////////////////////
    // north west (+9)
    for (int i=1; i<8; i++) {
	const auto sq = b.mailbox[index + 9*i];
	if (index + 9*i == ownKingLocation) continue;
	if (sq.type == PieceType::outOfBoard) {
	    break;
	} else if (sq.type == PieceType::empty) {
	    continue;
	} else if (sq.color == ownColor) {
	    break;
	} else if (sq.color == enemyColor) {
	    if (sq.type == PieceType::bishop) return true;
	    if (sq.type == PieceType::queen) return true;
	}
    }
    // south east (-9)
    for (int i=1; i<8; i++) {
	const auto sq = b.mailbox[index - 9*i];
	if (index - 9*i == ownKingLocation) continue;
	if (sq.type == PieceType::outOfBoard) {
	    break;
	} else if (sq.type == PieceType::empty) {
	    continue;
	} else if (sq.color == ownColor) {
	    break;
	} else if (sq.color == enemyColor) {
	    if (sq.type == PieceType::bishop) return true;
	    if (sq.type == PieceType::queen) return true;
	}
    }
    // north east (+11)
    for (int i=1; i<8; i++) {
	const auto sq = b.mailbox[index + 11*i];
	if (index + 11*i == ownKingLocation) continue;
	if (sq.type == PieceType::outOfBoard) {
	    break;
	} else if (sq.type == PieceType::empty) {
	    continue;
	} else if (sq.color == ownColor) {
	    break;
	} else if (sq.color == enemyColor) {
	    if (sq.type == PieceType::bishop) return true;
	    if (sq.type == PieceType::queen) return true;
	}
    }
    // south west (-11)
    for (int i=1; i<8; i++) {
	const auto sq = b.mailbox[index - 11*i];
	if (index - 11*i == ownKingLocation) continue;
	if (sq.type == PieceType::outOfBoard) {
	    break;
	} else if (sq.type == PieceType::empty) {
	    continue;
	} else if (sq.color == ownColor) {
	    break;
	} else if (sq.color == enemyColor) {
	    if (sq.type == PieceType::bishop) return true;
	    if (sq.type == PieceType::queen) return true;
	}
    }

    ///////////////////////////////////////////////////////
    //                      Knights                      //
    ///////////////////////////////////////////////////////
    if (b.mailbox[index + 21].type == PieceType::knight
	and b.mailbox[index + 21].color == enemyColor) return true;
    if (b.mailbox[index - 21].type == PieceType::knight
	and b.mailbox[index - 21].color == enemyColor) return true;
    if (b.mailbox[index + 19].type == PieceType::knight
	and b.mailbox[index + 19].color == enemyColor) return true;
    if (b.mailbox[index - 19].type == PieceType::knight
	and b.mailbox[index - 19].color == enemyColor) return true;
    if (b.mailbox[index + 12].type == PieceType::knight
	and b.mailbox[index + 12].color == enemyColor) return true;
    if (b.mailbox[index - 12].type == PieceType::knight
	and b.mailbox[index - 12].color == enemyColor) return true;
    if (b.mailbox[index + 8].type == PieceType::knight
	and b.mailbox[index + 8].color == enemyColor) return true;
    if (b.mailbox[index - 8].type == PieceType::knight
	and b.mailbox[index - 8].color == enemyColor) return true;

    ///////////////////////////////////////////////////////
    //                        King                       //
    ///////////////////////////////////////////////////////
    if (index + 10 == enemyKingLocation) return true;
    if (index - 10 == enemyKingLocation) return true;
    if (index +  1 == enemyKingLocation) return true;
    if (index -  1 == enemyKingLocation) return true;
    if (index + 11 == enemyKingLocation) return true;
    if (index - 11 == enemyKingLocation) return true;
    if (index +  9 == enemyKingLocation) return true;
    if (index -  9 == enemyKingLocation) return true;

    ///////////////////////////////////////////////////////
    //                       Pawns                       //
    ///////////////////////////////////////////////////////
    const int pawnAtt1 = (b.whiteToMove ? 11 : -11);
    const int pawnAtt2 = (b.whiteToMove ?  9 :  -9);
    if (b.mailbox[index - pawnAtt1].type == PieceType::pawn
	and b.mailbox[index - pawnAtt1].color == enemyColor) return true;
    if (b.mailbox[index - pawnAtt2].type == PieceType::pawn
	and b.mailbox[index - pawnAtt2].color == enemyColor) return true;

    return false;
}
