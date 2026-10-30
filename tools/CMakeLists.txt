add_executable(azalea_tune tune.cpp)
target_link_libraries(azalea_tune PRIVATE azalea_lib)
target_compile_options(azalea_tune PRIVATE
    -Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wsign-conversion)
