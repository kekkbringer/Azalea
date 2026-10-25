#include "engine.hpp"
#include "uci.hpp"

#include <iostream>

int main() {
    azalea::Engine engine;
    azalea::Uci uci(engine, std::cin, std::cout);
    uci.loop();

    return 0;
}
