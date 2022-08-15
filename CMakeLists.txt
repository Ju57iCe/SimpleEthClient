cmake_minimum_required(VERSION 3.8)

project(SimpleEthClient)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(WORKSPACE_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR})

add_subdirectory(${WORKSPACE_ROOT_DIR}/deps/googletest-1.8.0-hunter-p11)
add_subdirectory(${WORKSPACE_ROOT_DIR}/deps/fmt-6.1.2)
add_subdirectory(${WORKSPACE_ROOT_DIR}/deps/GSL-2.0.0)
add_subdirectory(${WORKSPACE_ROOT_DIR}/deps/c-ares-1.14.0)
add_subdirectory(${WORKSPACE_ROOT_DIR}/deps/yaml-cpp-yaml-cpp-0.6.2)
add_subdirectory(${WORKSPACE_ROOT_DIR}/deps/soralog-0.0.9)
add_subdirectory(${WORKSPACE_ROOT_DIR}/deps/hat-trie-0.6.0)
add_subdirectory(${WORKSPACE_ROOT_DIR}/deps/di-1.1.0)
add_subdirectory(${WORKSPACE_ROOT_DIR}/deps/lib-p2p-cpp)
add_subdirectory(${WORKSPACE_ROOT_DIR}/deps/solidity-0.8.15)

add_subdirectory(libs/Utils)
add_subdirectory(libs/hash-library)
add_subdirectory(tests)
add_subdirectory(src)