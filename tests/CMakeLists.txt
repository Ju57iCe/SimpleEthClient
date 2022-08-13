cmake_minimum_required(VERSION 3.14)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/../cmake)

include(CodeStyle)
include(GoogleTest)
include(FetchContent)

project(tests)

include_directories(${WORKSPACE_ROOT_DIR}/deps/googletest-1.8.0-hunter-p11/googletest/include)

# GoogleTest requires at least C++11
set(CMAKE_CXX_STANDARD 17)

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