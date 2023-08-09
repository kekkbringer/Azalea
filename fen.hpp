#pragma once

#include <string>

#include "board.hpp"

/******************************************************************************
 * Function that reads in a fen string and returns the corresponding board.
 */
GameState fen(std::string f);
