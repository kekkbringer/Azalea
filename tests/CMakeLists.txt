include(FetchContent)

FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG v3.16.1
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(Catch2)

add_executable(azalea_tests
    test_attacks.cpp
    test_board.cpp
    test_legality.cpp
    test_make_move.cpp
    test_move.cpp
    test_perft.cpp
    test_repetition.cpp
    test_unmake_move.cpp
    test_version.cpp
    test_zobrist.cpp
)
target_link_libraries(azalea_tests PRIVATE azalea_lib Catch2::Catch2WithMain)

include(Catch)
catch_discover_tests(azalea_tests)
