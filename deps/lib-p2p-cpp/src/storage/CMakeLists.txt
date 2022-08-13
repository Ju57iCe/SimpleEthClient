#
# Copyright Soramitsu Co., Ltd. All Rights Reserved.
# SPDX-License-Identifier: Apache-2.0
#

libp2p_add_library(p2p_sqlite sqlite.cpp)

target_include_directories(p2p_sqlite PRIVATE ${WORKSPACE_ROOT_DIR}/deps/sqlite_modern_cpp-3.2/hdr)

target_link_libraries(p2p_sqlite PRIVATE p2p_logger)
