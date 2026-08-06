#include "grm/config.hpp"
#include "grm/json_utils.hpp"
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <string_view>

namespace grm {

static fmt::OutputFormat parse_format_str(std::string_view str) {
  if (str == "human")
    return fmt::OutputFormat::Human;
  if (str == "markdown")
    return fmt::OutputFormat::Markdown;
  if (str == "json")
    return fmt::OutputFormat::Json;
  if (str == "jsonl" || str == "ndjson")
    return fmt::OutputFormat::JsonL;
  if (str == "plain")
    return fmt::OutputFormat::Plain;
  return fmt::OutputFormat::Auto;
}

static fmt::ColorMode parse_color_str(std::string_view str) {
  if (str == "always")
    return fmt::ColorMode::Always;
  if (str == "never")
    return fmt::ColorMode::Never;
  if (str == "auto")
    return fmt::ColorMode::Auto;
  return fmt::ColorMode::Auto;
}

static NameFormat parse_name_format_str(std::string_view str) {
  if (str == "fullname" || str == "full_name" || str == "full") {
    return NameFormat::Fullname;
  }
  return NameFormat::Username;
}

std::expected<Config, std::string>
Config::load(const std::filesystem::path &custom_path) {
  Config cfg;

  const char *home = std::getenv("HOME");
  if (!home) {
    return std::unexpected("HOME environment variable not set");
  }

  const std::filesystem::path home_path(home);

  const char *xdg_config = std::getenv("XDG_CONFIG_HOME");

  const std::filesystem::path config_dir =
      xdg_config ? std::filesystem::path(xdg_config) / "grm"
                 : home_path / ".config" / "grm";
  const std::filesystem::path lib_dir = home_path / ".local" / "lib" / "grm";

  const std::filesystem::path tgcli_cfg = home_path / ".tgcli" / "config.json";
  const std::filesystem::path grm_cfg = config_dir / "config.json";

  cfg.config_dir = config_dir;
  cfg.db_dir = lib_dir / "tdlib_db";

  std::filesystem::create_directories(cfg.config_dir);
  std::filesystem::create_directories(cfg.db_dir);

  std::filesystem::path active_cfg;
  if (!custom_path.empty()) {
    active_cfg = custom_path;
  } else if (std::filesystem::exists(grm_cfg)) {
    active_cfg = grm_cfg;
  } else if (std::filesystem::exists(tgcli_cfg)) {
    active_cfg = tgcli_cfg;
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
        if (auto fmt_str = parsed->get_string("format")) {
          cfg.default_format = parse_format_str(*fmt_str);
        }
        if (auto color_str = parsed->get_string("color")) {
          cfg.default_color_mode = parse_color_str(*color_str);
        }
        if (auto name_fmt_str = parsed->get_string("sender_name_format")) {
          cfg.default_name_format = parse_name_format_str(*name_fmt_str);
        } else if (auto legacy_name_fmt = parsed->get_string("name_format")) {
          cfg.default_name_format = parse_name_format_str(*legacy_name_fmt);
        }
      }
    }
  }

  // Environment variable overrides (GRM_FORMAT, GRM_COLOR, GRM_NAME_FORMAT)
  if (const char *env_fmt = std::getenv("GRM_FORMAT")) {
    if (env_fmt[0] != '\0') {
      cfg.default_format = parse_format_str(env_fmt);
    }
  }
  if (const char *env_color = std::getenv("GRM_COLOR")) {
    if (env_color[0] != '\0') {
      cfg.default_color_mode = parse_color_str(env_color);
    }
  }
  if (const char *env_name = std::getenv("GRM_SENDER_NAME_FORMAT")) {
    if (env_name[0] != '\0') {
      cfg.default_name_format = parse_name_format_str(env_name);
    }
  } else if (const char *env_name2 = std::getenv("GRM_NAME_FORMAT")) {
    if (env_name2[0] != '\0') {
      cfg.default_name_format = parse_name_format_str(env_name2);
    }
  }

  return cfg;
}

} // namespace grm
