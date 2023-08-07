#include "util.hpp"

#include <string>

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
