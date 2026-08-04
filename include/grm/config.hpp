#pragma once

#include "grm/logger.hpp"
#include <expected>
#include <filesystem>
#include <string>

namespace grm {

struct CliOptions {
  std::string phone;
  std::string code;
  std::filesystem::path custom_config_path;
  log::VerbosityLevel verbosity{log::VerbosityLevel::Normal};
  bool use_test_dc{false};
  bool help{false};
  bool version{false};
};

struct Config {
  int32_t api_id{27379307};
  std::string api_hash{"e001df71d7e35904a00f2a9a7a03d60e"};
  std::filesystem::path config_dir;
  std::filesystem::path db_dir;

  [[nodiscard]] static std::expected<Config, std::string>
  load(const std::filesystem::path &custom_path = {});
};

} // namespace grm
