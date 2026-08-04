#ifndef GRM_LOGGER_HPP
#define GRM_LOGGER_HPP

#include <string_view>

namespace grm::log {

enum class VerbosityLevel {
  Quiet = 0,
  Normal = 1,
  Verbose = 2,
  Debug = 3
};

void set_verbosity(VerbosityLevel level);
VerbosityLevel get_verbosity();

void info(std::string_view msg);
void warn(std::string_view msg);
void error(std::string_view msg);
void auth(std::string_view msg);
void debug(std::string_view msg);

} // namespace grm::log

#endif // GRM_LOGGER_HPP
