#pragma once

#include "grm/formatter.hpp"
#include "grm/logger.hpp"
#include <expected>
#include <filesystem>
#include <string>

namespace grm {

enum class NameFormat : std::uint8_t { Username, Fullname };

struct CliOptions {
  std::string phone;
  std::string code;
  std::filesystem::path custom_config_path;
  log::VerbosityLevel verbosity{log::VerbosityLevel::Normal};
  fmt::OutputFormat format{fmt::OutputFormat::Auto};
  fmt::ColorMode color_mode{fmt::ColorMode::Auto};
  NameFormat name_format{NameFormat::Username};
  bool use_test_dc{false};
  bool help{false};
  bool help_all{false};
  bool pretty{false};
  bool version{false};
  bool qr{false};
};

struct Config {
  int32_t api_id{27379307};
  std::string api_hash{"e001df71d7e35904a00f2a9a7a03d60e"};
  std::filesystem::path config_dir;
  std::filesystem::path db_dir;
  fmt::OutputFormat default_format{fmt::OutputFormat::Auto};
  fmt::ColorMode default_color_mode{fmt::ColorMode::Auto};
  NameFormat default_name_format{NameFormat::Username};

  [[nodiscard]] static std::expected<Config, std::string>
  load(const std::filesystem::path &custom_path = {});
};

} // namespace grm
