#include "grm/logger.hpp"
#include <iostream>
#include <mutex>

namespace grm::log {

static VerbosityLevel g_verbosity = VerbosityLevel::Normal;
static std::mutex g_log_mutex;

void set_verbosity(VerbosityLevel level) {
  std::scoped_lock lock(g_log_mutex);
  g_verbosity = level;
}

VerbosityLevel get_verbosity() {
  std::scoped_lock lock(g_log_mutex);
  return g_verbosity;
}

void info(std::string_view msg) {
  std::scoped_lock lock(g_log_mutex);
  if (g_verbosity >= VerbosityLevel::Normal) {
    std::cout << "[INFO] " << msg << '\n';
  }
}

void warn(std::string_view msg) {
  std::scoped_lock lock(g_log_mutex);
  if (g_verbosity >= VerbosityLevel::Normal) {
    std::cerr << "[WARN] " << msg << '\n';
  }
}

void error(std::string_view msg) {
  std::scoped_lock lock(g_log_mutex);
  if (g_verbosity >= VerbosityLevel::Quiet) {
    std::cerr << "[ERROR] " << msg << '\n';
  }
}

void auth(std::string_view msg) {
  std::scoped_lock lock(g_log_mutex);
  if (g_verbosity >= VerbosityLevel::Normal) {
    std::cout << "[AUTH] " << msg << '\n';
  }
}

void debug(std::string_view msg) {
  std::scoped_lock lock(g_log_mutex);
  if (g_verbosity >= VerbosityLevel::Verbose) {
    std::cout << "[DEBUG] " << msg << '\n';
  }
}

} // namespace grm::log
