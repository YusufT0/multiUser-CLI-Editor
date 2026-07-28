#pragma once
#include <deque>
#include <mutex>
#include <string>

class Logger {
public:
  static Logger &instance();
  void log(const std::string &message);
  std::deque<std::string> get_logs() const;
  static constexpr int MAX_LOGS = 3;

private:
  Logger() = default;
  mutable std::mutex mutex_;
  std::deque<std::string> logs_;
};
