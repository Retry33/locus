include(FetchContent)
FetchContent_Declare(googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG v1.15.2)
set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

add_executable(locus_tests detail/raw_storage_test.cpp)
target_link_libraries(locus_tests PRIVATE locus::core GTest::gtest_main)
target_compile_options(locus_tests PRIVATE -Wall -Wextra -Wpedantic -Werror)

option(LOCUS_ENABLE_SANITIZERS "Build tests with ASan and UBSan" ON)
if(LOCUS_ENABLE_SANITIZERS)
  target_compile_options(locus_tests PRIVATE
    -fsanitize=address,undefined -fno-omit-frame-pointer)
  target_link_options(locus_tests PRIVATE -fsanitize=address,undefined)
endif()

include(GoogleTest)
gtest_discover_tests(locus_tests)