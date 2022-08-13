#
# Copyright Soramitsu Co., Ltd. All Rights Reserved.
# SPDX-License-Identifier: Apache-2.0
#

include_directories(${WORKSPACE_ROOT_DIR}/deps/soralog-0.0.9/include)
include_directories(${WORKSPACE_ROOT_DIR}/deps/fmt-6.1.2/include)

libp2p_add_library(p2p_router
    router_impl.cpp
    )
target_link_libraries(p2p_router
    Boost::boost
    tsl_hat_trie
    p2p_peer_id
    )


libp2p_add_library(p2p_listener_manager
    listener_manager_impl.cpp
    )
target_link_libraries(p2p_listener_manager
    Boost::boost
    p2p_multiaddress
    p2p_peer_id
    p2p_logger
    )


libp2p_add_library(p2p_dialer
    dialer_impl.cpp
    )
target_link_libraries(p2p_dialer
    Boost::boost
    p2p_multiaddress
    p2p_multiselect
    p2p_peer_id
    p2p_logger
    )


libp2p_add_library(p2p_network
    network_impl.cpp
    )
target_link_libraries(p2p_network
    Boost::boost
    )

libp2p_add_library(p2p_transport_manager
    transport_manager_impl.cpp
    )
target_link_libraries(p2p_transport_manager
    p2p_multiaddress
    )

libp2p_add_library(p2p_connection_manager
    connection_manager_impl.cpp
    )
target_link_libraries(p2p_connection_manager
    Boost::boost
    )

libp2p_add_library(p2p_dnsaddr_resolver
    dnsaddr_resolver_impl.cpp
    )
target_link_libraries(p2p_dnsaddr_resolver
    Boost::boost
    p2p_cares
    )
