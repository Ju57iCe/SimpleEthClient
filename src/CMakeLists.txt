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
target_include_directories(${PROJECT_NAME} PUBLIC ${WORKSPACE_ROOT_DIR}/deps/lib-p2p-cpp/include)
target_include_directories(${PROJECT_NAME} PUBLIC ${WORKSPACE_ROOT_DIR}/deps/GSL-2.0.0/include)
target_include_directories(${PROJECT_NAME} PUBLIC ${WORKSPACE_ROOT_DIR}/deps/di-1.1.0/include)
target_include_directories(${PROJECT_NAME} PUBLIC ${WORKSPACE_ROOT_DIR}/deps/soralog-0.0.9/include)
target_include_directories(${PROJECT_NAME} PUBLIC ${WORKSPACE_ROOT_DIR}/deps/yaml-cpp-yaml-cpp-0.6.2/include)

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