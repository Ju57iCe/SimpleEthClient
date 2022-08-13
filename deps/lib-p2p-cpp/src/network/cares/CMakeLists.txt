#
# Copyright Soramitsu Co., Ltd. All Rights Reserved.
# SPDX-License-Identifier: Apache-2.0
#

include_directories(${WORKSPACE_ROOT_DIR}/deps/soralog-0.0.9/include)
include_directories(${WORKSPACE_ROOT_DIR}/deps/fmt-6.1.2/include)

libp2p_add_library(p2p_cares
    cares.cpp
    )
target_link_libraries(p2p_cares
    c-ares::cares
    ${CMAKE_THREAD_LIBS_INIT}
    )
