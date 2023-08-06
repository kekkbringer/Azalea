#pragma once

#include <string>

/******************************************************************************
 * This file include general functions to output debug and statistics
 * information.
 *****************************************************************************/


/******************************************************************************
 * Function that deletes the statistics file give in 'azalea::stat_file',
 * defaul name is 'stats.aza'.
 */
void deleteStatfile();

/******************************************************************************
 * Simple function that dumps a string given in 'text' to the statistics file
 * 'azalea::stat_file', default name is 'stats.aza'.
 */
void outputStats(const std::string text);

/******************************************************************************
 * Function that dumps boot message to statfile, containing version info
 */
void dumpBootMsg();
