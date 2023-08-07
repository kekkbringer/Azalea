#pragma once

#include <string>

#include "board.hpp"

/******************************************************************************
 * Function that reads in a fen string and returns the corresponding board.
 */
Board fen(std::string f);
