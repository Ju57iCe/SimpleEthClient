#pragma once

#include <libp2p/common/hexutil.hpp>
#include <libp2p/injector/kademlia_injector.hpp>
#include <libp2p/log/configurator.hpp>
#include <libp2p/log/sublogger.hpp>
#include <libp2p/multi/content_identifier_codec.hpp>

class Session;
struct Cmp;

extern std::set<std::shared_ptr<Session>, Cmp> sessions;
extern boost::optional<libp2p::peer::PeerId> self_id;

void handleOutgoingStream(
    libp2p::protocol::BaseProtocol::StreamResult stream_res) {
  if (not stream_res) {
    std::cerr << " ! outgoing connection failed: "
              << stream_res.error().message() << std::endl;
    return;
  }
  auto &stream = stream_res.value();

  // reject outgoing stream to themselves
  if (stream->remotePeerId().value() == self_id) {
    stream->reset();
    return;
  }

  std::cout << stream->remotePeerId().value().toBase58()
            << " + outgoing stream to "
            << stream->remoteMultiaddr().value().getStringAddress()
            << std::endl;

  auto session = std::make_shared<Session>(stream);
  if (auto [it, ok] = sessions.emplace(std::move(session)); ok) {
    (*it)->read(sessions);
  }
}

void handleIncomingStream(
    libp2p::protocol::BaseProtocol::StreamResult stream_res) {
  if (not stream_res) {
    std::cerr << " ! incoming connection failed: "
              << stream_res.error().message() << std::endl;
    return;
  }
  auto &stream = stream_res.value();

  // reject incoming stream with themselves
  if (stream->remotePeerId().value() == self_id) {
    stream->reset();
    return;
  }

  std::cout << stream->remotePeerId().value().toBase58()
            << " + incoming stream from "
            << stream->remoteMultiaddr().value().getStringAddress()
            << std::endl;

  auto session = std::make_shared<Session>(stream);
  if (auto [it, ok] = sessions.emplace(std::move(session)); ok) {
    (*it)->read(sessions);
  }
}