include(FetchContent)

FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG v3.16.1
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(Catch2)

add_executable(azalea_tests test_version.cpp)
target_link_libraries(azalea_tests PRIVATE azalea_lib Catch2::Catch2WithMain)

include(Catch)
catch_discover_tests(azalea_tests)
