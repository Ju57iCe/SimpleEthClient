/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include <iostream>
#include <memory>
#include <set>
#include <vector>
#include <tuple>

#include "node_config.h"
#include "crypto/KeyGenerator.h"
#include "session.h"
#include "stream_handlers.h"

#include <boost/beast.hpp>

#include <libp2p/common/hexutil.hpp>
#include <libp2p/injector/kademlia_injector.hpp>
#include <libp2p/log/configurator.hpp>
#include <libp2p/log/sublogger.hpp>
#include <libp2p/multi/content_identifier_codec.hpp>

//===================== GLOBALS ======================================
// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
boost::optional<libp2p::peer::PeerId> self_id;

// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
std::set<std::shared_ptr<Session>, Cmp> sessions;

libp2p::protocol::kademlia::Config kademlia_config;

std::shared_ptr<libp2p::protocol::kademlia::Kademlia> kademlia;
// Key for group of chat
libp2p::protocol::kademlia::ContentId content_id("meet me here");

auto injector = libp2p::injector::makeHostInjector(
      // libp2p::injector::useKeyPair(kp), // Use predefined keypair
      libp2p::injector::makeKademliaInjector(
          libp2p::injector::useKademliaConfig(kademlia_config)));
auto &scheduler = injector.create<libp2p::protocol::Scheduler &>();

std::shared_ptr<libp2p::Host> host = nullptr;
//====================================================================

std::shared_ptr<soralog::LoggingSystem> init_logging()
{
  return std::make_shared<soralog::LoggingSystem>(
      std::make_shared<soralog::ConfiguratorFromYAML>(
          // Original LibP2P logging config
          std::make_shared<libp2p::log::Configurator>(),
          // Additional logging config for application
          config::logger_config));
}

void configure_logging(std::shared_ptr<soralog::LoggingSystem>& logging_system)
{
  auto r = logging_system->configure();
  if (not r.message.empty()) {
    (r.has_error ? std::cerr : std::cout) << r.message << std::endl;
  }
  if (r.has_error) {
    exit(EXIT_FAILURE);
  }
}

void init_kademlia_config()
{
  kademlia_config.randomWalk.enabled = true;
  kademlia_config.randomWalk.interval = std::chrono::seconds(300);
  kademlia_config.requestConcurency = 20;
}

void find_providers()
{
  [[maybe_unused]] auto res1 = kademlia->findProviders(
      content_id, 0,
      [&](libp2p::outcome::result<std::vector<libp2p::peer::PeerInfo>>
              res) {
        scheduler
            .schedule(libp2p::protocol::scheduler::toTicks(
                          kademlia_config.randomWalk.interval),
                      find_providers)
            .detach();

        if (not res) {
          std::cerr << "Cannot find providers: " << res.error().message()
                    << std::endl;
          return;
        }

        auto &providers = res.value();
        for (auto &provider : providers) {
          host->newStream(provider, "/chat/1.1.0", handleOutgoingStream);
        }
      });
};

void provide()
{
  [[maybe_unused]] auto res =
      kademlia->provide(content_id, not kademlia_config.passiveMode);

  scheduler
      .schedule(libp2p::protocol::scheduler::toTicks(
                    kademlia_config.randomWalk.interval),
                provide)
      .detach();
};

// Asynchronous transmit data from standard input to peers, that's privided
// same content id
void read_from_console(boost::asio::posix::stream_descriptor& in,
                        std::array<uint8_t, 1 << 12>& buffer)
{
  in.async_read_some(boost::asio::buffer(buffer), [&](auto ec, auto size) {
    auto i = std::find_if(buffer.begin(), buffer.begin() + size + 1,
                          [](auto c) { return c == '\n'; });

    if (i != buffer.begin() + size + 1) {
      auto out = std::make_shared<std::vector<uint8_t>>();
      out->assign(buffer.begin(), buffer.begin() + size);

      for (const auto &session : sessions) {
        session->write(out, sessions);
      }
    }
    read_from_console(in, buffer);
  });
}

std::vector<libp2p::peer::PeerInfo> bootstrap_nodes_fn()
{
      std::vector<std::string> addresses = config::addresses;

      std::unordered_map<libp2p::peer::PeerId,
                         std::vector<libp2p::multi::Multiaddress>>
          addresses_by_peer_id;

      for (auto &address : addresses) {
        auto ma = libp2p::multi::Multiaddress::create(address).value();
        auto peer_id_base58 = ma.getPeerId().value();
        auto peer_id = libp2p::peer::PeerId::fromBase58(peer_id_base58).value();

        addresses_by_peer_id[std::move(peer_id)].emplace_back(std::move(ma));
      }

      std::vector<libp2p::peer::PeerInfo> v;
      v.reserve(addresses_by_peer_id.size());
      for (auto &i : addresses_by_peer_id) {
        v.emplace_back(libp2p::peer::PeerInfo{
            .id = i.first, .addresses = {std::move(i.second)}});
      }

      return v;
}

int main(int argc, char *argv[]) {
  // prepare log system
  auto logging_system = init_logging();
  configure_logging(logging_system);

  libp2p::log::setLoggingSystem(logging_system);
  if (std::getenv("TRACE_DEBUG") != nullptr) {
    libp2p::log::setLevelOfGroup("main", soralog::Level::TRACE);
  } else {
    libp2p::log::setLevelOfGroup("main", soralog::Level::ERROR);
  }

  // resulting PeerId should be
  // 12D3KooWEgUjBV5FJAuBSoNMRYFRHjV7PjZwRQ7b43EKX9g7D6xV
  libp2p::crypto::KeyPair kp = {
      // clang-format off
      .publicKey = {{
        .type = libp2p::crypto::Key::Type::Ed25519,
        .data = libp2p::common::unhex(config::hex_bootstrap_pub_key).value()
      }},
      .privateKey = {{
        .type = libp2p::crypto::Key::Type::Ed25519,
        .data = libp2p::common::unhex(config::hex_bootstrap_priv_key).value()
      }},
      // clang-format on
  };

  try {
    if (argc < 2) {
      std::cerr << "Needs one argument - address" << std::endl;
      exit(EXIT_FAILURE);
    }

    auto bootstrap_nodes = bootstrap_nodes_fn();

    auto ma = libp2p::multi::Multiaddress::create(argv[1]).value();  // NOLINT

    auto io = injector.create<std::shared_ptr<boost::asio::io_context>>();

    host = injector.create<std::shared_ptr<libp2p::Host>>();

    self_id = host->getId();

    std::cerr << self_id->toBase58() << " * started" << std::endl;

    init_kademlia_config();

    kademlia = injector.create<std::shared_ptr<libp2p::protocol::kademlia::Kademlia>>();

    // Handle streams for observed protocol
    host->setProtocolHandler("/chat/1.0.0", handleIncomingStream);
    host->setProtocolHandler("/chat/1.1.0", handleIncomingStream);

    io->post([&] {
      auto listen = host->listen(ma);
      if (not listen) {
        std::cerr << "Cannot listen address " << ma.getStringAddress().data()
                  << ". Error: " << listen.error().message() << std::endl;
        std::exit(EXIT_FAILURE);
      }

      for (auto &bootstrap_node : bootstrap_nodes) {
        kademlia->addPeer(bootstrap_node, true);
      }

      host->start();

      auto cid = libp2p::multi::ContentIdentifierCodec::decode(content_id.data)
                     .value();
      auto peer_id =
          libp2p::peer::PeerId::fromHash(cid.content_address).value();

      [[maybe_unused]] auto res = kademlia->findPeer(peer_id, [&](auto) {
        // Say to world about his providing
        provide();

        // Ask provider from world
        find_providers();

        kademlia->start();
      });
    });

    boost::asio::posix::stream_descriptor in(*io, ::dup(STDIN_FILENO));
    std::array<uint8_t, 1 << 12> buffer{};

    read_from_console(in, buffer);

    boost::asio::signal_set signals(*io, SIGINT, SIGTERM);
    signals.async_wait(
        [&io](const boost::system::error_code &, int) { io->stop(); });
    io->run();
  } catch (const std::exception &e) {
    std::cerr << "Exception: " << e.what() << std::endl;
    exit(EXIT_FAILURE);
  }

  // auto tup = GenerateKey();
  // std::cout << std::get<0>(tup) << std::endl << std::get<1>(tup) << std::endl;
  exit(EXIT_SUCCESS);
}
