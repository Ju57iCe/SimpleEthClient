#pragma once


#include <iostream>
#include <memory>
#include <set>
#include <vector>
#include <tuple>

#include "node_config.h"
#include "Crypto/KeyGenerator.h"
#include "session.h"

#include <boost/beast.hpp>

#include <libp2p/common/hexutil.hpp>
#include <libp2p/injector/kademlia_injector.hpp>
#include <libp2p/log/configurator.hpp>
#include <libp2p/log/sublogger.hpp>
#include <libp2p/multi/content_identifier_codec.hpp>

class Session;

struct Cmp {
  bool operator()(const std::shared_ptr<Session> &lhs,
                  const std::shared_ptr<Session> &rhs) const;
};

class Session : public std::enable_shared_from_this<Session> {
 public:
  explicit Session(std::shared_ptr<libp2p::connection::Stream> stream);

  bool read(std::set<std::shared_ptr<Session>, Cmp>& sessions);
  bool write(const std::shared_ptr<std::vector<uint8_t>> &buffer, std::set<std::shared_ptr<Session>, Cmp>& sessions);
  void close(std::set<std::shared_ptr<Session>, Cmp>& sessions);
  bool operator<(const Session &other);

 private:
  std::shared_ptr<libp2p::connection::Stream> stream_;
  std::shared_ptr<std::vector<uint8_t>> incoming_;
};
