// TestConfig validates [TECH-001] and [FUNC-001]
#include "grm/config.hpp"
#include <cassert>
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

int main() {
  std::cout << "Running Config unit tests...\n";
  test_config_defaults();
  std::cout << "All Config tests passed successfully.\n";
  return 0;
}
