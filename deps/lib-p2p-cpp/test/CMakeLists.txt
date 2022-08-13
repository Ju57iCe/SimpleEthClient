#
# Copyright Soramitsu Co., Ltd. All Rights Reserved.
# SPDX-License-Identifier: Apache-2.0
#

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${PROJECT_SOURCE_DIR})

include_directories(${WORKSPACE_ROOT_DIR}/deps/GSL-2.0.0/include)

add_subdirectory(acceptance)
add_subdirectory(deps)
add_subdirectory(libp2p)
add_subdirectory(testutil)
