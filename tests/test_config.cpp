#include "grm/config.hpp"
#include <cassert>
#include <cstdlib>
#include <iostream>

void test_config_defaults() {
  auto cfg = grm::Config::load();
  assert(cfg.has_value());
  assert(cfg->api_id == 27379307);
  assert(!cfg->api_hash.empty());
  assert(!cfg->config_dir.empty());
  assert(!cfg->db_dir.empty());
  std::cout << "[PASS] test_config_defaults\n";
}

void test_config_env_overrides() {
  setenv("GRM_FORMAT", "json", 1);
  setenv("GRM_COLOR", "never", 1);

  auto cfg = grm::Config::load();
  assert(cfg.has_value());
  assert(cfg->default_format == grm::fmt::OutputFormat::Json);
  assert(cfg->default_color_mode == grm::fmt::ColorMode::Never);

  unsetenv("GRM_FORMAT");
  unsetenv("GRM_COLOR");

  std::cout << "[PASS] test_config_env_overrides\n";
}

int main() {
  std::cout << "Running Config unit tests...\n";
  test_config_defaults();
  test_config_env_overrides();
  std::cout << "All Config tests passed successfully.\n";
  return 0;
}
