#include "logger.hpp"

Logger &Logger::instance() {
  static Logger inst;
  return inst;
}
void Logger::log(const std::string &message) {
  std::lock_guard<std::mutex> lock(mutex_);
  logs_.push_back(message);
  while ((int)logs_.size() > MAX_LOGS)
    logs_.pop_front();
}
std::deque<std::string> Logger::get_logs() const {
  std::lock_guard<std::mutex> lock(mutex_);
  return logs_;
}
