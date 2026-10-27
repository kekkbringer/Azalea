add_library(azalea_lib STATIC
    board.cpp
    uci.cpp
    version.cpp
    movegeneration.cpp
    engine.cpp
    search.cpp
    perft.cpp
    attacks.cpp
    eval.cpp
    search_helper.cpp
    transposition.cpp
)
target_include_directories(azalea_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# Hardware popcount: without it std::popcount compiles to a library call (__popcountdi2).
# PUBLIC so the tests compile the inline bitboard helpers in the headers the same way.
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang" AND CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64|AMD64")
    target_compile_options(azalea_lib PUBLIC -mpopcnt)
endif()

add_executable(azalea main.cpp)
target_link_libraries(azalea PRIVATE azalea_lib)

foreach(target IN ITEMS azalea_lib azalea)
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(${target} PRIVATE
            -Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wsign-conversion)
    endif()

    if(AZALEA_ENABLE_SANITIZERS)
        target_compile_options(${target} PRIVATE
            -fsanitize=address,undefined -fno-omit-frame-pointer)
        # PUBLIC so anything linking azalea_lib (e.g. the tests) pulls in the sanitizer runtime.
        target_link_options(${target} PUBLIC
            -fsanitize=address,undefined -fno-omit-frame-pointer)
    endif()
endforeach()
