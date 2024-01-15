#include "util.hpp"

#include <string>
#include <iostream>
#include <sys/poll.h>

#include "board.hpp"

/******************************************************************************
 * Function that converts a square in algebraic notation, e.g. 'e4' to its
 * corresponding index on the bitboard representation,
 */
int toIndex(std::string s) {
    int index = 0;
    index += (s[1]-'1')*8;
    index += 7-(s[0]-'a');
    return index;
}

/******************************************************************************
 * Function that converts a index on the bitboard to its corresponding square
 * in algebraic notation, e.g. "e4"
 */
constexpr std::string_view algsq[64]
	    = {"h1", "g1", "f1", "e1", "d1", "c1", "b1", "a1",
	       "h2", "g2", "f2", "e2", "d2", "c2", "b2", "a2",
	       "h3", "g3", "f3", "e3", "d3", "c3", "b3", "a3",
	       "h4", "g4", "f4", "e4", "d4", "c4", "b4", "a4",
	       "h5", "g5", "f5", "e5", "d5", "c5", "b5", "a5",
	       "h6", "g6", "f6", "e6", "d6", "c6", "b6", "a6",
	       "h7", "g7", "f7", "e7", "d7", "c7", "b7", "a7",
	       "h8", "g8", "f8", "e8", "d8", "c8", "b8", "a8"};
std::string_view toString(int index) {
    return algsq[index];
}

/******************************************************************************
 * Function that converts a 'Move' to a string corresponding to algeraic
 * notation, e.g. "e2e4" or "a7a8q".
 */
std::string toString(const Move& m) {
    std::string alg = "";
    alg += toString(m.from);
    alg += toString(m.to);
    if (m.promoPiece != ' ') alg += m.promoPiece;
    return alg;
}

/******************************************************************************
 * Overload of function for printing 'Move's to std::ostreams.
 */
std::ostream& operator<<(std::ostream& os, const Move& m) {
    return os << toString(m);
}

/******************************************************************************
 * listen to std::cin, if 'stop' was given its returns true.
*/
bool listenForStop() {
    struct pollfd fds;
    int ret;
    fds.fd = 0;
    fds.events = POLLIN;
    ret = poll(&fds, 1, 0);
    if (ret == 1) {
	std::string command;
	std::getline(std::cin, command);
	if (command == "stop") return true;
	if (command == "isready") std::cout << "readyok" << std::endl;
    }
    return false;
}

/******************************************************************************
 * Returns the score in cp or the mate score.
 */
std::string scoreOrMate(const int score) {
    std::string res = "";
    if (score >= -azalea::CHECKMATE - 100) {
	return "mate " + std::to_string((-score - azalea::CHECKMATE + 1)/2);
    }
    if (score <= azalea::CHECKMATE + 100) {
	return "mate -" + std::to_string((score - azalea::CHECKMATE + 1)/2);
    }
    return "cp " + std::to_string(score/10);
}
