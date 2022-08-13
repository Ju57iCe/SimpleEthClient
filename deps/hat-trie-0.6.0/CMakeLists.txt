cmake_minimum_required(VERSION 3.1)

project(tsl_hat_trie)

add_library(tsl_hat_trie INTERFACE)
# Use tsl::hat_trie as target, more consistent with other libraries conventions (Boost, Qt, ...)
add_library(tsl::hat_trie ALIAS tsl_hat_trie)

target_include_directories(tsl_hat_trie INTERFACE
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
    "$<INSTALL_INTERFACE:include/tsl>")
# target_sources(tsl_hat_trie PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include/tsl/array-hash/array_growth_policy.h"
#                                       "${CMAKE_CURRENT_SOURCE_DIR}/include/tsl/array-hash/array_hash.h"
#                                       "${CMAKE_CURRENT_SOURCE_DIR}/include/tsl/array-hash/array_map.h"
#                                       "${CMAKE_CURRENT_SOURCE_DIR}/include/tsl/array-hash/array_set.h"
#                                       "${CMAKE_CURRENT_SOURCE_DIR}/include/tsl/htrie_hash.h"
#                                       "${CMAKE_CURRENT_SOURCE_DIR}/include/tsl/htrie_map.h"
#                                       "${CMAKE_CURRENT_SOURCE_DIR}/include/tsl/htrie_set.h")

if(${CMAKE_VERSION} VERSION_GREATER "3.7")
    # Only available since version 3.8
    target_compile_features(tsl_hat_trie INTERFACE cxx_std_11)
endif()


install(TARGETS tsl_hat_trie EXPORT LIB_EXPORTS)
install(EXPORT LIB_EXPORTS
        NAMESPACE tsl::
        DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/export
)