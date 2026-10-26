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
)
target_include_directories(azalea_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

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
