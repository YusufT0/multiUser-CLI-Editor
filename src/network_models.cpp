#include "network_models.hpp"
#include "asio/buffer.hpp"
#include "asio/error.hpp"
#include "asio/error_code.hpp"
#include "asio/io_context.hpp"
#include "asio/ip/tcp.hpp"
#include "asio/write.hpp"
#include <asio.hpp>
#include <iostream>
#include <logger.hpp>
#include <memory>
#include <string>
using asio::ip::tcp;

tcp_session::tcp_session(tcp::socket socket) : socket_(std::move(socket)) {}
tcp::socket &tcp_session::socket() { return socket_; }
void tcp_session::start() { do_read(); }
void tcp_session::do_read() {
  read_buf_.resize(1024);
  socket_.async_read_some(
      asio::buffer(read_buf_),
      [self = shared_from_this()](const asio::error_code &err,
                                  size_t bytes_transeffer) {
        if (!err) {
          self->do_read();
        }
      });
}
void tcp_session::send_buffer(const std::string &data) {
  uint32_t len = static_cast<uint32_t>(data.size());
  std::vector<char> packet(sizeof(len) + data.size());
  std::memcpy(packet.data(), &len, sizeof(len));
  std::memcpy(packet.data() + sizeof(len), data.data(), data.size());
  auto buf = std::make_shared<std::vector<char>>(std::move(packet));
  asio::async_write(socket_, asio::buffer(*buf),
                    [this, self = shared_from_this(), buf](
                        const asio::error_code &err, size_t bytes_transferred) {
                      if (err) {
                        asio::error_code ec;
                        socket_.close(ec);
                        Logger::instance().log("Send failed." + err.message());
                      }
                    });
}
tcp_server::tcp_server(asio::io_context &io_context, unsigned short port)
    : io_context_(io_context),
      acceptor_(io_context, tcp::endpoint(tcp::v4(), port)) {}

void tcp_server::start() { start_accept(); }

void tcp_server::stop() {
  asio::error_code ec;
  acceptor_.close(ec);
  for (auto &session : sessions_) {
    session->socket().close(ec);
  }
  sessions_.clear();
  io_context_.stop();
};
void tcp_server::start_accept() {
  auto new_session = std::make_shared<tcp_session>(tcp::socket(io_context_));
  acceptor_.async_accept(new_session->socket(),
                         [this, new_session](const asio::error_code &err) {
                           if (!err) {
                             Logger::instance().log("Client connected!");
                             sessions_.push_back(new_session);
                             new_session->start();
                             if (!latest_data_.empty()) {
                               new_session->send_buffer(latest_data_);
                             }
                           }
                           if (err != asio::error::operation_aborted) {
                             start_accept();
                           }
                         });
}
void tcp_server::broadcast(const std::string &data) {
  latest_data_ = data;
  for (auto it = sessions_.begin(); it != sessions_.end();) {
    if ((*it)->socket().is_open()) {
      (*it)->send_buffer(data);
      ++it;

    } else {
      Logger::instance().log("Client disconnected.");
      it = sessions_.erase(it);
    }
  }
}

asio::io_context &tcp_server::get_io_context() { return io_context_; }

tcp_client::tcp_client(asio::io_context &io_context, const std::string &host,
                       unsigned short port)
    : io_context_(io_context), socket_(io_context) {
  tcp::resolver resolver(io_context);
  auto endpoints = resolver.resolve(host, std::to_string(port));
  asio::connect(socket_, endpoints);
  Logger::instance().log("[Network] Connected to host.");
}

void tcp_client::start_receiving() { do_read_header(); }

void tcp_client::do_read_header() {
  if (stopped_)
    return;
  read_buf_.resize(4);
  asio::async_read(socket_, asio::buffer(read_buf_),
                   [this](const asio::error_code &err, size_t /*len*/) {
                     if (err) {
                       if (err != asio::error::eof)
                         Logger::instance().log("[Network] Read error: " +
                                                err.message());
                       return;
                     }
                     uint32_t body_length = 0;
                     std::memcpy(&body_length, read_buf_.data(), 4);
                     do_read_body(body_length);
                   });
}

void tcp_client::do_read_body(uint32_t body_length) {
  if (stopped_)
    return;
  read_buf_.resize(body_length);
  asio::async_read(
      socket_, asio::buffer(read_buf_),
      [this](const asio::error_code &err, size_t /*len*/) {
        if (err) {
          if (err != asio::error::eof)
            Logger::instance().log("[Network] Read error: " + err.message());
          return;
        }
        {
          std::lock_guard<std::mutex> lock(buffer_mutex_);
          latest_buffer_.assign(read_buf_.begin(), read_buf_.end());
          new_data_ = true;
        }
        do_read_header();
      });
}

bool tcp_client::has_new_data() const { return new_data_; }

std::string tcp_client::get_latest_buffer() {
  std::lock_guard<std::mutex> lock(buffer_mutex_);
  new_data_ = false;
  return latest_buffer_;
}

void tcp_client::stop() {
  stopped_ = true;
  asio::error_code ec;
  socket_.close(ec);
  io_context_.stop();
}
