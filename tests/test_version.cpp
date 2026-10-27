#include "version.hpp"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("version() returns the project version", "[version]") {
    REQUIRE(azalea::version() == "0.1.5");
}
