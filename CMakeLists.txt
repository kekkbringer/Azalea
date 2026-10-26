cmake_minimum_required(VERSION 3.25)

project(azalea VERSION 0.1.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(AZALEA_BUILD_TESTS "Build the Azalea test suite" ON)
option(AZALEA_ENABLE_SANITIZERS "Build with AddressSanitizer and UndefinedBehaviorSanitizer" OFF)

add_subdirectory(src)

if(AZALEA_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
