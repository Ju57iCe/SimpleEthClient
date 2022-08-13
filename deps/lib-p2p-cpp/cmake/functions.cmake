function(disable_clang_tidy target)
  set_target_properties(${target} PROPERTIES
      C_CLANG_TIDY ""
      CXX_CLANG_TIDY ""
      )
endfunction()

function(addtest test_name)
  add_executable(${test_name} ${ARGN})
  addtest_part(${test_name} ${ARGN})
  target_link_libraries(${test_name} GTest::gmock_main)
  add_test(
      NAME ${test_name}
      COMMAND $<TARGET_FILE:${test_name}>
  )
  set_target_properties(${test_name} PROPERTIES
      RUNTIME_OUTPUT_DIRECTORY ${WORKSPACE_ROOT_DIR}/build/deps/lib-p2p-cpp/test_bin
      ARCHIVE_OUTPUT_PATH ${WORKSPACE_ROOT_DIR}/build/deps/lib-p2p-cpp/test_lib
      LIBRARY_OUTPUT_PATH ${WORKSPACE_ROOT_DIR}/build/deps/lib-p2p-cpp/test_lib
      )
  disable_clang_tidy(${test_name})
  set_property(GLOBAL APPEND PROPERTY TEST_TARGETS ${test_name})
endfunction()

function(addtest_part test_name)
  if (POLICY CMP0076)
    cmake_policy(SET CMP0076 NEW)
  endif ()
  target_sources(${test_name} PUBLIC
      ${ARGN}
      )
  target_link_libraries(${test_name}
      GTest::gtest
      )
endfunction()

# conditionally applies flag. If flag is supported by current compiler, it will be added to compile options.
function(add_flag flag)
  check_cxx_compiler_flag(${flag} FLAG_${flag})
  if (FLAG_${flag} EQUAL 1)
    add_compile_options(${flag})
  endif ()
endfunction()

function(compile_proto_to_cpp PROTO_LIBRARY_NAME PB_H PB_CC PROTO)
  if (NOT Protobuf_INCLUDE_DIR)
    get_target_property(Protobuf_INCLUDE_DIR protobuf::libprotobuf INTERFACE_INCLUDE_DIRECTORIES)
  endif()
  if (NOT Protobuf_INCLUDE_DIR)
    message(FATAL_ERROR "Protobuf_INCLUDE_DIR is empty")
  endif ()

  if (NOT Protobuf_PROTOC_EXECUTABLE)
    get_target_property(Protobuf_PROTOC_EXECUTABLE protobuf::protoc IMPORTED_LOCATION_RELEASE)
    set(PROTOBUF_DEPENDS protobuf::protoc)
  endif()
  if (NOT Protobuf_PROTOC_EXECUTABLE)
    get_target_property(Protobuf_PROTOC_EXECUTABLE protobuf::protoc IMPORTED_LOCATION_DEBUG)
    set(PROTOBUF_DEPENDS protobuf::protoc)
  endif()
  if (NOT Protobuf_PROTOC_EXECUTABLE)
    get_target_property(Protobuf_PROTOC_EXECUTABLE protobuf::protoc IMPORTED_LOCATION)
    set(PROTOBUF_DEPENDS protobuf::protoc)
  endif()
  if (NOT Protobuf_PROTOC_EXECUTABLE)
    message(FATAL_ERROR "Protobuf_PROTOC_EXECUTABLE is empty")
  endif ()

  get_filename_component(PROTO_ABS "${PROTO}" ABSOLUTE)
  # get relative (to CMAKE_BINARY_DIR) path of current proto file

  file(RELATIVE_PATH SCHEMA_REL "${CMAKE_BINARY_DIR}/src" "${CMAKE_CURRENT_BINARY_DIR}")

  set(SCHEMA_OUT_DIR ${CMAKE_BINARY_DIR}/deps/lib-p2p-cpp/pb/${PROTO_LIBRARY_NAME}/generated)
  file(MAKE_DIRECTORY ${SCHEMA_OUT_DIR})

  string(REGEX REPLACE "\\.proto$" ".pb.h" GEN_PB_HEADER ${PROTO})
  string(REGEX REPLACE "\\.proto$" ".pb.cc" GEN_PB ${PROTO})

  set(GEN_COMMAND ${Protobuf_PROTOC_EXECUTABLE})
  set(GEN_ARGS ${Protobuf_INCLUDE_DIR})

  set(OUT_HPP ${SCHEMA_OUT_DIR}/${SCHEMA_REL}/${GEN_PB_HEADER})
  set(OUT_CPP ${SCHEMA_OUT_DIR}/${SCHEMA_REL}/${GEN_PB})

  
  string(REPLACE "../deps/lib-p2p-cpp/src/" "" OUT_HPP_PROCESSED ${SCHEMA_OUT_DIR}/${SCHEMA_REL}/${GEN_PB_HEADER})
  string(REPLACE "../deps/lib-p2p-cpp/src/" "" OUT_CPP_PROCESSED ${SCHEMA_OUT_DIR}/${SCHEMA_REL}/${GEN_PB})

  set(GENERATION_DIR ${SCHEMA_OUT_DIR}/${SCHEMA_REL})

  message(STATUS "=========================")
  message(STATUS "CMAKE_BINARY_DIR ${CMAKE_BINARY_DIR}")
  message(STATUS "CMAKE_CURRENT_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}")
  message(STATUS "GEN ARGS ${GEN_ARGS}")
  message(STATUS "OUT_HPP_PROCESSED ${OUT_HPP_PROCESSED}")
  message(STATUS "OUT_CPP_PROCESSED ${OUT_CPP_PROCESSED}")

  message(STATUS "PROJECT_SOURCE_DIR/src ${PROJECT_SOURCE_DIR}/src")
  message(STATUS "SCHEMA_OUT_DIR ${SCHEMA_OUT_DIR}")
  message(STATUS "PROTO_ABS ${PROTO_ABS}")


  add_custom_command(
      OUTPUT ${OUT_HPP_PROCESSED} ${OUT_CPP_PROCESSED}
      COMMAND ${GEN_COMMAND}
      ARGS -I${PROJECT_SOURCE_DIR}/src -I${GEN_ARGS} --cpp_out=${SCHEMA_OUT_DIR} ${PROTO_ABS}
      WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
      DEPENDS ${PROTOBUF_DEPENDS} ${PROTO_ABS}
      VERBATIM)

  set(${PB_H} ${OUT_HPP_PROCESSED} PARENT_SCOPE)
  set(${PB_CC} ${OUT_CPP_PROCESSED} PARENT_SCOPE)
endfunction()

add_custom_target(generated
    COMMENT "Building generated files..."
    )

function(add_proto_library NAME)
  set(SOURCES "")
  foreach (PROTO IN ITEMS ${ARGN})
    compile_proto_to_cpp(${NAME} H C ${PROTO})
    list(APPEND SOURCES ${H} ${C})
  endforeach ()

  add_library(${NAME}
      ${SOURCES}
      )
  target_link_libraries(${NAME}
      protobuf::libprotobuf
      )
  target_include_directories(${NAME} PUBLIC
      # required for common targets
      $<BUILD_INTERFACE:${WORKSPACE_ROOT_DIR}/deps/lib-p2p-cpp/pb/${NAME}>
      # required for compiling proto targets
      $<BUILD_INTERFACE:${WORKSPACE_ROOT_DIR}/deps/lib-p2p-cpp/pb/${NAME}/generated>
      )

  disable_clang_tidy(${NAME})
  libp2p_install(${NAME})

  add_dependencies(generated ${NAME})
endfunction()
