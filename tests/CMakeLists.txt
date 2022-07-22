cmake_minimum_required(VERSION 3.14)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/../cmake)

include(CodeStyle)
include(GoogleTest)
include(FetchContent)

project(tests)

# GoogleTest requires at least C++11
set(CMAKE_CXX_STANDARD 17)

FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/609281088cfefc76f9d0ce82e1ff6c30cc3591e5.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

if(NOT ${IGNORE_CODE_STYLE_CHECKS})
    set_code_style(rlp_tests lower_case ignore_headers "${extra_flags}")
    set_code_style(mpt_tests lower_case ignore_headers "${extra_flags}")
endif()

#================== RLP ========================
add_executable(
  rlp_tests
  rlp_tests.cpp
)

target_link_libraries(rlp_tests PRIVATE utils)
target_link_libraries(rlp_tests PRIVATE gtest_main)

gtest_discover_tests(rlp_tests)

#================== MPT ========================
add_executable(
  mpt_tests
  mpt_tests.cpp
)

target_link_libraries(mpt_tests PRIVATE utils)
target_link_libraries(mpt_tests PRIVATE gtest_main)

gtest_discover_tests(mpt_tests)