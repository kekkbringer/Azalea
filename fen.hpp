#pragma once

#include <string>

#include "board.hpp"
#include "zobrist.hpp"

/******************************************************************************
 * Function that reads in a fen string and returns the corresponding board.
 */
GameState fen(std::string f, const zobristKeys& zobrist);

/******************************************************************************
 * Returns FEN string of current game state
 */
std::string toFen (const GameState& gs);
