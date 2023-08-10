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
	    {19, 51, 1316, 6554, 187508, 874713, 26196477}},
    {"2rQ4/7B/nN2bK1N/8/r7/1k6/3n2q1/4B2R w - - 0 1",
	    {43, 2364, 93032, 4709184}},
    {"2r5/4N3/2b5/1k6/4n3/8/8/4K1B1 w - - 0 1",
	    {16, 403, 6814, 184797, 3107625}}
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
