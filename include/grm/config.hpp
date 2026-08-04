#pragma once

#include <expected>
#include <filesystem>
#include <string>

namespace grm {

struct Config {
  int32_t api_id{27379307};
  std::string api_hash{"e001df71d7e35904a00f2a9a7a03d60e"};
  std::filesystem::path config_dir;
  std::filesystem::path db_dir;

  [[nodiscard]] static std::expected<Config, std::string> load();
};

} // namespace grm
