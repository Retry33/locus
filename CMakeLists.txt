cmake_minimum_required(VERSION 3.25...4.4)

project(locus
    VERSION "0.1.0" 
    DESCRIPTION "Cache misses are a moral failing"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if (NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Debug)
endif()

add_library(locus INTERFACE)
add_library(locus::core ALIAS locus)
target_include_directories(locus INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_features(locus INTERFACE cxx_std_20)

option(LOCUS_BUILD_TESTS "Build Locus tests" ${PROJECT_IS_TOP_LEVEL})

if(LOCUS_BUILD_TESTS)
  enable_testing()
  add_subdirectory(tests)
endif()