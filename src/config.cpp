#include "grm/config.hpp"
#include "grm/json_utils.hpp"
#include <cstdlib>
#include <fstream>
#include <sstream>

namespace grm {

std::expected<Config, std::string>
Config::load(const std::filesystem::path &custom_path) {
  Config cfg;

  const char *home = std::getenv("HOME");
  if (!home) {
    return std::unexpected("HOME environment variable not set");
  }

  const std::filesystem::path home_path(home);

  // Check legacy ~/.tgcli/config.json first if available
  const std::filesystem::path tgcli_cfg = home_path / ".tgcli" / "config.json";
  const std::filesystem::path grm_dir = home_path / ".config" / "grm";
  const std::filesystem::path grm_cfg = grm_dir / "config.json";

  cfg.config_dir = grm_dir;
  cfg.db_dir = grm_dir / "tdlib_db";

  std::filesystem::create_directories(cfg.config_dir);
  std::filesystem::create_directories(cfg.db_dir);

  std::filesystem::path active_cfg;
  if (!custom_path.empty()) {
    active_cfg = custom_path;
  } else if (std::filesystem::exists(tgcli_cfg)) {
    active_cfg = tgcli_cfg;
  } else {
    active_cfg = grm_cfg;
  }

  if (std::filesystem::exists(active_cfg)) {
    std::ifstream file(active_cfg);
    if (file) {
      std::ostringstream ss;
      ss << file.rdbuf();
      auto parsed = JsonValue::parse(ss.str());
      if (parsed) {
        if (auto id = parsed->get_int("api_id")) {
          cfg.api_id = static_cast<int32_t>(*id);
        } else if (auto legacy_id = parsed->get_int("app_id")) {
          cfg.api_id = static_cast<int32_t>(*legacy_id);
        }
        if (auto hash = parsed->get_string("api_hash")) {
          cfg.api_hash = *hash;
        } else if (auto legacy_hash = parsed->get_string("app_hash")) {
          cfg.api_hash = *legacy_hash;
        }
      }
    }
  }

  return cfg;
}

} // namespace grm
