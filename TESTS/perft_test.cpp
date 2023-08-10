#include <catch2/catch.hpp>

#define CATCH_CONFIG_NOSTDOUT

#include <vector>
#include <string>

#include "board.hpp"
#include "movegenerator.hpp"
#include "fen.hpp"
#include "util.hpp"

struct sperft {
    std::string fenstring;
    std::vector<unsigned long long int> perftResults;
};

std::vector<sperft> perft1 = {
    {"7k/8/8/8/8/8/1K6/R7 w - - 0 1",
	    {21, 57, 1080, 5592, 110367, 544919}},
    {"7k/8/8/8/8/8/1K6/RR6 w - - 0 1",
	    {19, 51, 1316, 6554, 187508, 874713, 26196477}}
};

TEST_CASE("Testing perft...", "[perft]") {
    for (const auto& pos: perft1) {
	SECTION ("fen " + pos.fenstring) {
	    GameState gs = fen (pos.fenstring);
	    int depth = 1;
	    for (const auto& res: pos.perftResults) {
		REQUIRE(res == perft(gs, depth));
		depth++;
	    }
	}
    }
}
