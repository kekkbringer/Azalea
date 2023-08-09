#pragma once

#include <cstdint>

typedef uint64_t bitb;

namespace azalea {
    /**************************************************************************
     * Version section
     */
    constexpr int majorVersion = 0;
    constexpr int minorVersion = 0;
    constexpr int patchVersion = 0;

    /**************************************************************************
     * Statistics section
     *
     * statisitcs - azalea dumps statistics to disk if true
     * statfile   - file name for statistics
     */
    constexpr bool statistics = true;
    constexpr auto statfile = "stats.aza";

    /**************************************************************************
     * Move generation
     *
     * maxMoves - maximum number of moves possible
     */
    constexpr int maxMoves = 218;
}
