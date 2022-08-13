#
# Copyright Soramitsu Co., Ltd. All Rights Reserved.
# SPDX-License-Identifier: Apache-2.0
#

include_directories(${WORKSPACE_ROOT_DIR}/deps/GSL-2.0.0/include)

add_subdirectory(host)

addtest(all_muxers_acceptance_test
    muxer.cpp
    )
target_link_libraries(all_muxers_acceptance_test
    p2p_yamux
    p2p_mplex
    p2p_multiaddress
    p2p_plaintext
    p2p_tcp
    p2p_testutil
    p2p_key_marshaller
    p2p_identity_manager
    p2p_literals
    p2p_asio_scheduler_backend
    p2p_basic_scheduler
    )
