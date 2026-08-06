#include "grm/logger.hpp"
#include <iostream>
#include <mutex>

namespace grm::log {

namespace {

struct LoggerState {
  VerbosityLevel verbosity{VerbosityLevel::Normal};
  std::mutex mutex;
};

LoggerState &get_state() {
  static LoggerState state;
  return state;
}

} // namespace

void set_verbosity(VerbosityLevel level) {
  auto &state = get_state();
  std::scoped_lock lock(state.mutex);
  state.verbosity = level;
}

VerbosityLevel get_verbosity() {
  auto &state = get_state();
  std::scoped_lock lock(state.mutex);
  return state.verbosity;
}

void info(std::string_view msg) {
  auto &state = get_state();
  std::scoped_lock lock(state.mutex);
  if (state.verbosity >= VerbosityLevel::Normal) {
    std::cout << "[INFO] " << msg << '\n';
  }
}

void warn(std::string_view msg) {
  auto &state = get_state();
  std::scoped_lock lock(state.mutex);
  if (state.verbosity >= VerbosityLevel::Normal) {
    std::cerr << "[WARN] " << msg << '\n';
  }
}

void error(std::string_view msg) {
  auto &state = get_state();
  std::scoped_lock lock(state.mutex);
  if (state.verbosity >= VerbosityLevel::Quiet) {
    std::cerr << "[ERROR] " << msg << '\n';
  }
}

void auth(std::string_view msg) {
  auto &state = get_state();
  std::scoped_lock lock(state.mutex);
  if (state.verbosity >= VerbosityLevel::Normal) {
    std::cout << "[AUTH] " << msg << '\n';
  }
}

void verbose(std::string_view msg) {
  auto &state = get_state();
  std::scoped_lock lock(state.mutex);
  if (state.verbosity >= VerbosityLevel::Verbose) {
    std::cout << "[VERBOSE] " << msg << '\n';
  }
}

void debug(std::string_view msg) {
  auto &state = get_state();
  std::scoped_lock lock(state.mutex);
  if (state.verbosity >= VerbosityLevel::Debug) {
    std::cout << "[DEBUG] " << msg << '\n';
  }
}

} // namespace grm::log
