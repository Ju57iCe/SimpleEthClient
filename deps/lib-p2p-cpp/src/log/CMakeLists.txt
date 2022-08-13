#
# Copyright Soramitsu Co., Ltd. All Rights Reserved.
# SPDX-License-Identifier: Apache-2.0
#

include_directories(${WORKSPACE_ROOT_DIR}/deps/yaml-cpp-yaml-cpp-0.6.2/include)
include_directories(${WORKSPACE_ROOT_DIR}/deps/di-1.1.0/include)

libp2p_add_library(p2p_logger
    logger.cpp
    configurator.cpp
    )
target_link_libraries(p2p_logger
    soralog
    fallback_configurator
    configurator_yaml
    )
