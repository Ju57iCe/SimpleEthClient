#pragma once

#include <iostream>
#include <memory>
#include <set>
#include <vector>
#include <tuple>
#include <string>

#include "node_config.h"
#include "crypto/KeyGenerator.h"
#include "session.h"

#include <boost/beast.hpp>

#include <libp2p/common/hexutil.hpp>
#include <libp2p/injector/kademlia_injector.hpp>
#include <libp2p/log/configurator.hpp>
#include <libp2p/log/sublogger.hpp>
#include <libp2p/multi/content_identifier_codec.hpp>


class ClientApp
{
public:
    ClientApp(std::string multiaddress);
    ~ClientApp();
    void run();
private:

    std::shared_ptr<soralog::LoggingSystem> init_logging();
    void configure_logging(std::shared_ptr<soralog::LoggingSystem>& logging_system);
    void read_from_console(boost::asio::posix::stream_descriptor& in,
                            std::array<uint8_t, 1 << 12>& buffer);

    static void handleOutgoingStream(
        libp2p::protocol::BaseProtocol::StreamResult stream_res);
    static void handleIncomingStream(
        libp2p::protocol::BaseProtocol::StreamResult stream_res);
    std::vector<libp2p::peer::PeerInfo> bootstrap_nodes_fn();

    static auto& get_kademlia_config();
    static auto& get_kademlia();
    static auto& get_injector();
    static auto& get_scheduler();
    static auto& get_self_id();
    static auto& get_sessions();
    static auto& get_host();

    static void find_providers();
    static void provide();
private:
    //===================== GLOBALS ======================================
    std::shared_ptr<libp2p::protocol::kademlia::Kademlia> kademlia;
    // Key for group of chat
    static libp2p::protocol::kademlia::ContentId content_id;

    std::shared_ptr<libp2p::Host> host = nullptr;

    std::string multiaddress;
    //====================================================================
};