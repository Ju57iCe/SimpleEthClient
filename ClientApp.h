#pragma once

#include <iostream>
#include <memory>
#include <set>
#include <vector>
#include <tuple>
#include <string>

#include "node_config.h"
#include "Crypto/KeyGenerator.h"
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

    std::shared_ptr<soralog::LoggingSystem> InitLogging();
    void ConfigureLogging(std::shared_ptr<soralog::LoggingSystem>& logging_system);
    void ReadFromConsole(boost::asio::posix::stream_descriptor& in,
                            std::array<uint8_t, 1 << 12>& buffer);

    static void HandleOutgoingStream(
        libp2p::protocol::BaseProtocol::StreamResult stream_res);
    static void HandleIncomingStream(
        libp2p::protocol::BaseProtocol::StreamResult stream_res);
    std::vector<libp2p::peer::PeerInfo> BootstrapNodesFn();

    static auto& GetKademliaConfig();
    static auto& GetKademlia();
    static auto& GetInjector();
    static auto& GetScheduler();
    static auto& GetSelfId();
    static auto& GetSessions();
    static auto& GetHost();

    static void FindProviders();
    static void Provide();
private:
    //===================== GLOBALS ======================================
    std::shared_ptr<libp2p::protocol::kademlia::Kademlia> kademlia;
    // Key for group of chat
    static libp2p::protocol::kademlia::ContentId content_id;

    std::shared_ptr<libp2p::Host> host = nullptr;

    std::string multiaddress;
    //====================================================================
};