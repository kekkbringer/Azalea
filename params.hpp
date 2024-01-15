#pragma once

#include <cstdint>
#include <limits>
#include <cstddef>

typedef uint64_t bitb;

// bitscan macros
// BSF = Bit Scan Forward
#define BSF(x) (__builtin_ffsll(x) - 1)
// BSB = Bit Scan Backward
#define BSB(x) (63 - __builtin_clzll(x))

// population count macro
#define POP(x) (__builtin_popcountll(x))

namespace azalea {
    /**************************************************************************
     * Version section
     */
    constexpr int majorVersion = 1;
    constexpr int minorVersion = 0;
    constexpr int patchVersion = 1;
    constexpr auto addVersion = "";

    /**************************************************************************
     * Statistics section
     *
     * statisitcs - azalea dumps statistics to disk if true
     * statfile   - file name for statistics
     */
    constexpr bool statistics = true;
    constexpr auto statfile = "stats100.aza";

    /**************************************************************************
     * Integer constants
     */
    constexpr int MININT = std::numeric_limits<int>::min() + 50;
    constexpr int MAXINT = std::numeric_limits<int>::max() - 50;
    constexpr int CHECKMATE = -1'000'000;

    /**************************************************************************
     * Search parameter
    */
    constexpr int maxDepth = 100;
    constexpr int aspirationWindowSize = 520; // in millipawns
    constexpr int deltaMargin = 2000; // 2 pawns
    constexpr int repHistMaxPly = 1024;

    /**************************************************************************
     * Move generation
     *
     * maxMoves - maximum number of moves possible
     */
    constexpr int maxMoves = 218;
}

constexpr size_t ttsize = 1024*1024*4;
