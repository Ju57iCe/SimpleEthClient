#include "session.h"


Session::Session(std::shared_ptr<libp2p::connection::Stream> stream)
: stream_(std::move(stream))
, incoming_(std::make_shared<std::vector<uint8_t>>(1 << 12))
{
}

bool Cmp::operator()(const std::shared_ptr<Session> &lhs,
                     const std::shared_ptr<Session> &rhs) const {
  return *lhs < *rhs;
}

bool Session::read(std::set<std::shared_ptr<Session>, Cmp>& sessions) {
    if (stream_->isClosedForRead()) {
        close(sessions);
        return false;
    }

    stream_->readSome(
        gsl::span(incoming_->data(), static_cast<ssize_t>(incoming_->size())),
        incoming_->size(),
        [self = shared_from_this(),
            &sessions](libp2p::outcome::result<size_t> result) {
            if (not result) {
            self->close(sessions);
            std::cout << self->stream_->remotePeerId().value().toBase58()
                        << " - closed at reading" << std::endl;
            return;
            }
            std::cout << self->stream_->remotePeerId().value().toBase58() << " > "
                    << std::string(self->incoming_->begin(),
                                    self->incoming_->begin()
                                        + static_cast<ssize_t>(result.value()));
            std::cout.flush();
            self->read(sessions);
        });
    return true;
}

bool Session::write(const std::shared_ptr<std::vector<uint8_t>> &buffer, std::set<std::shared_ptr<Session>, Cmp>& sessions) {
    if (stream_->isClosedForWrite()) {
      close(sessions);
      return false;
    }

    stream_->write(
        gsl::span(buffer->data(), static_cast<ssize_t>(buffer->size())),
        buffer->size(),
        [self = shared_from_this(),
         buffer,
         &sessions](libp2p::outcome::result<size_t> result) {
          if (not result) {
            self->close(sessions);
            std::cout << self->stream_->remotePeerId().value().toBase58()
                      << " - closed at writting" << std::endl;
            return;
          }
          std::cout << self->stream_->remotePeerId().value().toBase58() << " < "
                    << std::string(buffer->begin(),
                                   buffer->begin()
                                       + static_cast<ssize_t>(result.value()));
          std::cout.flush();
        });
    return true;
  }

void Session::close(std::set<std::shared_ptr<Session>, Cmp>& sessions) {
    stream_->close([self = shared_from_this()](auto) {});
    sessions.erase(shared_from_this());
}

bool Session::operator<(const Session &other) {
    return stream_->remotePeerId().value()
        < other.stream_->remotePeerId().value();
}