#include "ClientApp.h"


libp2p::protocol::kademlia::ContentId ClientApp::content_id("meet me here");

ClientApp::ClientApp(std::string address)
: multiaddress(address)
{
}

ClientApp::~ClientApp()
{
}

auto& ClientApp::get_kademlia_config()
{
    static libp2p::protocol::kademlia::Config kademlia_config;
    kademlia_config.randomWalk.enabled = true;
    kademlia_config.randomWalk.interval = std::chrono::seconds(300);
    kademlia_config.requestConcurency = 20;
    return kademlia_config;
}

auto& ClientApp::get_injector()
{
    static auto injector = libp2p::injector::makeHostInjector(
      libp2p::injector::makeKademliaInjector(
          libp2p::injector::useKademliaConfig(get_kademlia_config())));
    return injector;
}

auto& ClientApp::get_kademlia()
{
    static auto kademlia =
        get_injector()
            .create<std::shared_ptr<libp2p::protocol::kademlia::Kademlia>>();
    
    return kademlia;
}

auto& ClientApp::get_scheduler()
{
    static auto& scheduler = get_injector().create<libp2p::protocol::Scheduler &>();
    return scheduler;
}

auto& ClientApp::get_self_id()
{
    static boost::optional<libp2p::peer::PeerId> self_id;
    return self_id;
}

auto& ClientApp::get_sessions()
{
    static std::set<std::shared_ptr<Session>, Cmp> sessions;
    return sessions;
}

auto& ClientApp::get_host()
{
    static auto host = get_injector().create<std::shared_ptr<libp2p::Host>>();
    return host;
}

std::shared_ptr<soralog::LoggingSystem> ClientApp::init_logging()
{
    return std::make_shared<soralog::LoggingSystem>(
        std::make_shared<soralog::ConfiguratorFromYAML>(
            // Original LibP2P logging config
            std::make_shared<libp2p::log::Configurator>(),
            // Additional logging config for application
            config::logger_config));
}

void ClientApp::configure_logging(std::shared_ptr<soralog::LoggingSystem>& logging_system)
{
    auto r = logging_system->configure();
    if (not r.message.empty()) {
        (r.has_error ? std::cerr : std::cout) << r.message << std::endl;
    }
    if (r.has_error) {
        exit(EXIT_FAILURE);
    }

    libp2p::log::setLoggingSystem(logging_system);
    if (std::getenv("TRACE_DEBUG") != nullptr) {
        libp2p::log::setLevelOfGroup("main", soralog::Level::TRACE);
    } else {
        libp2p::log::setLevelOfGroup("main", soralog::Level::ERROR);
    }
}

void ClientApp::find_providers()
{
    [[maybe_unused]] auto res1 = get_kademlia()->findProviders(
        content_id, 0,
        [&](libp2p::outcome::result<std::vector<libp2p::peer::PeerInfo>>
                res) {
            get_scheduler()
                .schedule(libp2p::protocol::scheduler::toTicks(
                            get_kademlia_config().randomWalk.interval),
                        find_providers)
                .detach();

            if (not res) {
            std::cerr << "Cannot find providers: " << res.error().message()
                        << std::endl;
            return;
            }

            auto &providers = res.value();
            for (auto &provider : providers) {
                get_host()->newStream(provider, "/chat/1.1.0", ClientApp::handleOutgoingStream);
            }
        });
};

void ClientApp::provide()
{
    [[maybe_unused]] auto res =
        get_kademlia()->provide(content_id, not get_kademlia_config().passiveMode);

    auto& scheduler = get_scheduler();
    scheduler.schedule(libp2p::protocol::scheduler::toTicks(
                        get_kademlia_config().randomWalk.interval),
                    provide)
        .detach();
};

// Asynchronous transmit data from standard input to peers, that's privided
// same content id
void ClientApp::read_from_console(boost::asio::posix::stream_descriptor& in,
                        std::array<uint8_t, 1 << 12>& buffer)
{
    in.async_read_some(boost::asio::buffer(buffer), [&](auto ec, auto size) {
        auto i = std::find_if(buffer.begin(), buffer.begin() + size + 1,
                            [](auto c) { return c == '\n'; });

        if (i != buffer.begin() + size + 1) {
        auto out = std::make_shared<std::vector<uint8_t>>();
        out->assign(buffer.begin(), buffer.begin() + size);

        for (const auto &session : get_sessions()) {
            session->write(out, get_sessions());
        }
        }
        read_from_console(in, buffer);
    });
}

std::vector<libp2p::peer::PeerInfo> ClientApp::bootstrap_nodes_fn()
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

void ClientApp::run()
{
    // prepare log system
    auto logging_system = init_logging();
    configure_logging(logging_system);

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

    auto& injector = get_injector();

    try {
        auto bootstrap_nodes = bootstrap_nodes_fn();

        auto ma = libp2p::multi::Multiaddress::create(multiaddress).value();  // NOLINT

        auto io = injector.create<std::shared_ptr<boost::asio::io_context>>();

        host = injector.create<std::shared_ptr<libp2p::Host>>();

        auto& self_id = get_self_id();
        self_id = host->getId();

        std::cerr << self_id->toBase58() << " * started" << std::endl;



        kademlia = injector.create<std::shared_ptr<libp2p::protocol::kademlia::Kademlia>>();

        // Handle streams for observed protocol
        host->setProtocolHandler("/chat/1.0.0", this->handleIncomingStream);
        host->setProtocolHandler("/chat/1.1.0", this->handleIncomingStream);

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

void ClientApp::handleOutgoingStream(
    libp2p::protocol::BaseProtocol::StreamResult stream_res) {
  if (not stream_res) {
    std::cerr << " ! outgoing connection failed: "
              << stream_res.error().message() << std::endl;
    return;
  }
  auto &stream = stream_res.value();

  // reject outgoing stream to themselves
  if (stream->remotePeerId().value() == get_self_id()) {
    stream->reset();
    return;
  }

  std::cout << stream->remotePeerId().value().toBase58()
            << " + outgoing stream to "
            << stream->remoteMultiaddr().value().getStringAddress()
            << std::endl;

  auto session = std::make_shared<Session>(stream);
  if (auto [it, ok] = get_sessions().emplace(std::move(session)); ok) {
    (*it)->read(get_sessions());
  }
}

void ClientApp::handleIncomingStream(
    libp2p::protocol::BaseProtocol::StreamResult stream_res) {
  if (not stream_res) {
    std::cerr << " ! incoming connection failed: "
              << stream_res.error().message() << std::endl;
    return;
  }
  auto &stream = stream_res.value();

  // reject incoming stream with themselves
  if (stream->remotePeerId().value() == get_self_id()) {
    stream->reset();
    return;
  }

  std::cout << stream->remotePeerId().value().toBase58()
            << " + incoming stream from "
            << stream->remoteMultiaddr().value().getStringAddress()
            << std::endl;

  auto session = std::make_shared<Session>(stream);
  if (auto [it, ok] = get_sessions().emplace(std::move(session)); ok) {
    (*it)->read(get_sessions());
  }
}