#include "version.hpp"

#include <iostream>

int main() {
    std::cout << "Azalea " << azalea::version() << '\n';
    return 0;
}
