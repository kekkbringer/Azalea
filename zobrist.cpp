#include "zobrist.hpp"

#include <iostream>
#include <random>

zobristKeys initZobrist() {
    std::mt19937_64 gen64;

    zobristKeys zobrist;

    // one key per square per piece:
    // pawn, knight, bishop, rook, queen, king = 6 pieces
    // together with 64 square = 6*64 = 384
    // FOR EACH COLOR
    // white
    for (int i=0; i<6; i++) {
	for (int j=0; j<64; j++) {
	    zobrist.pieceSquare[0][i][j] = gen64();
	}
    }
    // black
    for (int i=0; i<6; i++) {
	for (int j=0; j<64; j++) {
	    zobrist.pieceSquare[1][i][j] = gen64();
	}
    }

    // one key for side to move
    zobrist.sideToMove = gen64();

    // four for castling rights
    zobrist.castling[0] = gen64();
    zobrist.castling[1] = gen64();
    zobrist.castling[2] = gen64();
    zobrist.castling[3] = gen64();

    // eight for en passant file
    for (int i=0; i<8; i++) zobrist.epFile[i] = gen64();

    return zobrist;
}
