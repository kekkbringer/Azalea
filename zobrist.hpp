#pragma once

#include "params.hpp"

struct zobristKeys {
    uint64_t pieceSquare[2][6][64];
    uint64_t sideToMove; // set if black to move
    uint64_t castling[4]; // w short, w long, b short, b long
    uint64_t epFile[8];
};
void checkLinearIndependence(const int n, const zobristKeys z);
zobristKeys initZobrist();

