
#ifndef EDITOR_HPP
#define EDITOR_HPP
#include "asio/io_context.hpp"
#include "models.hpp"
#include "network_models.hpp"
#include "view_service.hpp"
#include <changefilemodal.hpp>
#include <memory>
#include <string>
#include <thread>
class Editor {
private:
  bool is_host = false;
  std::unique_ptr<tcp_server> server_;
  std::unique_ptr<tcp_client> client_;
  std::thread network_thread_;
  std::shared_ptr<asio::io_context> io_context_;
  bool running_ = false;
  std::string path;
  GapBuffer gap_buffer;
  Highlight highligter;
  bool DEBUG_GAP;
  ChangeFileModal changefileModal;
  ViewService view_service;
  Editor()
      : DEBUG_GAP(false),
        view_service(ViewService::TER_END_X, ViewService::TER_END_Y) {}
  ~Editor() = default;
  void process_input();

public:
  static Editor &get_instance() {
    static Editor instance;
    return instance;
  }
  std::string get_filename() const { return path; }
  Editor(const Editor &) = delete; // We do not want copy constructors. Do not
                                   // create a new editor by copying an old one.
  Editor &operator=(const Editor &) =
      delete; // Do not overwrite an existing editor with an old one.
  ChangeFileModal &get_change_file_modal() { return changefileModal; }
  void init(const std::string &p);
  void start_writing();
  void start_host(int port);
  void start_client(const std::string &host, int port);
  void broadcast_buffer();
  void client_loop();
  void shutdown();
  std::string get_buffer_text();
};

#endif
