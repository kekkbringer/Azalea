#include <string>
#include <fstream>
#include <cstdio>

#include "params.hpp"

#include "statistics.hpp"


/******************************************************************************
 * Function that deletes the statistics file give in 'azalea::statfile',
 * defaul name is 'stats.aza'.
 */
void deleteStatfile() {
    std::remove(azalea::statfile);
} // end of deleteStatfile

/******************************************************************************
 * Simple function that dumps a string given in 'text' to the statistics file
 * 'azalea::stat_file', default name is 'stats.aza'.
 */
void outputStats(const std::string text) {
    std::ofstream file;
    file.open(azalea::statfile, std::ios_base::app);
    file << text;
    file.close();
} // end of outputStats

/******************************************************************************
 * Function that dumps boot message to statfile, containing version info
 */
void dumpBootMsg() {
    std::ofstream file;
    file.open(azalea::statfile, std::ios_base::app);
    file << "booting Azalea " << azalea::majorVersion << "."
		      << azalea::minorVersion << "."
		      << azalea::patchVersion << " <3\n";
    file.close();
}
