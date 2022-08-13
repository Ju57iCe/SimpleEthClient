#
# Copyright Soramitsu Co., Ltd. All Rights Reserved.
# SPDX-License-Identifier: Apache-2.0
#
include_directories(${WORKSPACE_ROOT_DIR}/deps/soralog-0.0.9/include)
include_directories(${WORKSPACE_ROOT_DIR}/deps/yaml-cpp-yaml-cpp-0.6.2/include)

add_executable(example_simple
    main.cpp
    logging_object.cpp
    )
target_include_directories(example_simple
    PRIVATE ${CMAKE_SOURCE_DIR}/include
    )
target_link_libraries(example_simple
    soralog::soralog
    soralog::yaml
    soralog::fallback
    pthread
    )
