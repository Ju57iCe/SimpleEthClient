cmake_minimum_required(VERSION 3.4.3)

set(LIB_NAME utils)

set(CMAKE_BUILD_TYPE Debug)

file(GLOB_RECURSE SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.c"
                          "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
                          "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
                          "${CMAKE_CURRENT_SOURCE_DIR}/*.hpp")
add_library(${LIB_NAME} ${SOURCES})

if(NOT ${IGNORE_CODE_STYLE_CHECKS})
    set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/../../cmake)
    include(CodeStyle)
    set_code_style(${LIB_NAME} lower_case "ignore_headers" "${extra_flags}")
endif()

# Add definitions
set_target_properties(${LIB_NAME} PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
    POSITION_INDEPENDENT_CODE ON
    WINDOWS_EXPORT_ALL_SYMBOLS ON
    DEBUG_POSTFIX ""
)

target_include_directories(${LIB_NAME} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${WORKSPACE_ROOT_DIR}/deps/solidity-0.8.15/
)

target_link_libraries(${LIB_NAME} PRIVATE
    solutil
    )