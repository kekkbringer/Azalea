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
	    {16, 403, 6814, 184797, 3107625}},
    {"8/8/1KP2r2/2N2k2/8/5b2/8/8 w - - 0 1",
	    {14, 300, 3691, 80741, 913857, 21202020}},
    {"8/8/1KP2r2/2N2k2/8/5b2/4p3/3N4 w - - 0 1",
	    {18, 456, 7416, 197697, 2982477, 83852561}},
    {"r3k2r/8/8/8/8/8/8/R3K2R w KQkq - 0 1",
	    {26, 568, 13744, 314346, 7594526}},
    {"r3k2r/8/8/4n3/4N3/8/8/R3K2R w - - 0 1",
	    {32, 856, 24657, 694502, 20138062}},
    {"r3k2r/8/8/4n3/4N3/8/8/R3K2R w KQkq - 0 1",
	    {34, 970, 28709, 829170, 24223780}},
    {"rnbqkbnr/p7/8/8/8/8/7P/RNBQKBNR w KQkq - 0 1",
	    {44, 1657, 66516, 2617473, 107596964}},
    {"5k2/4p3/8/3P4/8/8/8/2K5 b - - 0 1",
	    {6, 38, 274, 2115, 15539, 120333, 878314, 6972639}},
    {"3r1k2/4p3/8/3P4/8/8/3K4/8 w - - 0 1",
	    {9, 117, 884, 13241, 99022}},
    {"8/4p3/8/K2P3r/8/8/8/7k w - - 0 1",
	    {5, 75, 550, 9137, 61644, 1080145}},
    {"8/8/8/K7/1R3pk1/8/4P3/8 w - - 0 1",
	    {18, 111, 2048, 13078, 247686, 1635283}},
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
