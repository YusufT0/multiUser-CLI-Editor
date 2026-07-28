#pragma once

#include "asio/io_context.hpp"
#include <asio.hpp>
#include <memory>
#include <mutex>
#include <string>
#include <vector>
using asio::ip::tcp;
class tcp_session : public std::enable_shared_from_this<tcp_session> {
public:
  tcp_session(tcp::socket socket);
  tcp::socket &socket();
  void send_buffer(const std::string &data);
  void start();

private:
  void do_read();
  tcp::socket socket_;
  std::vector<char> read_buf_;
};
class tcp_server {
public:
  tcp_server(asio::io_context &io_context, unsigned short port);
  void start();
  void stop();
  void broadcast(const std::string &data);
  asio::io_context &get_io_context();

private:
  void start_accept();
  asio::io_context &io_context_;
  tcp::acceptor acceptor_;
  std::vector<std::shared_ptr<tcp_session>> sessions_;
  std::string latest_data_;
};

class tcp_client {
public:
  tcp_client(asio::io_context &io_context, const std::string &host,
             unsigned short port);
  void start_receiving();
  bool has_new_data() const;
  std::string get_latest_buffer();
  void stop();

private:
  void do_read_header();
  void do_read_body(uint32_t body_len);

  asio::io_context &io_context_;
  tcp::socket socket_;
  std::vector<char> read_buf_;
  std::mutex buffer_mutex_;
  std::string latest_buffer_;
  bool new_data_ = false;
  bool stopped_ = false;
};
