cmake_minimum_required(VERSION 3.8)

project(SimpleEthClient)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(IGNORE_CODE_STYLE_CHECKS OFF)
set(CMAKE_BUILD_TYPE Debug)

FIND_PACKAGE(Boost COMPONENTS program_options REQUIRED )

file(GLOB_RECURSE SRC_LIST "${CMAKE_CURRENT_SOURCE_DIR}/*.c"
                          "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
                          "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
                          "${CMAKE_CURRENT_SOURCE_DIR}/*.hpp")

add_executable(${PROJECT_NAME} ${SRC_LIST})

if(NOT ${IGNORE_CODE_STYLE_CHECKS})
    set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/../cmake)
    include(CodeStyle)
    set_code_style(${PROJECT_NAME} lower_case ignore_headers "${extra_flags}")
endif()

target_include_directories(${PROJECT_NAME} PUBLIC ${BOOST_INCLUDE_DIRS})
target_include_directories(${PROJECT_NAME} PUBLIC ${ROOT_DIR}/libs)

target_link_libraries(${PROJECT_NAME} PRIVATE
    utils
    hash_library
    p2p_basic_host
    p2p_default_network
    p2p_peer_repository
    p2p_inmem_address_repository
    p2p_inmem_key_repository
    p2p_inmem_protocol_repository
    p2p_literals
    p2p_kademlia
    asio_scheduler
    p2p_multiaddress
    Boost::program_options
)