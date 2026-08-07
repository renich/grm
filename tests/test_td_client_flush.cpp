// TestTdClientFlush validates TdClient shutdown flush timing and lifecycle safety
#include "grm/config.hpp"
#include "grm/td_client.hpp"
#include <chrono>
#include <cstdlib>
#include <iostream>
#include <string>

static void check(bool condition, const std::string &msg) {
  if (!condition) {
    std::cerr << "Assertion failed: " << msg << std::endl;
    std::exit(1);
  }
}

void test_td_client_flush_timing() {
  grm::Config cfg{};
  grm::TdClient client(cfg);

  auto start_res = client.start();
  check(start_res.has_value(), "TdClient started successfully");

  auto t_start = std::chrono::steady_clock::now();
  client.stop();
  auto t_end = std::chrono::steady_clock::now();

  auto elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(t_end - t_start).count();
  
  // Verify that TdClient::stop shuts down cleanly and quickly (< 1000ms)
  check(elapsed_ms < 1000, "TdClient::stop shut down cleanly (< 1000ms)");
  std::cout << "[PASS] test_td_client_flush_timing (elapsed: " << elapsed_ms << "ms)\n";
}

void test_td_client_double_stop_idempotency() {
  grm::Config cfg{};
  grm::TdClient client(cfg);

  auto start_res = client.start();
  check(start_res.has_value(), "TdClient started successfully");

  client.stop();
  // Second call to stop should be a fast no-op since client is already stopped
  auto t_start = std::chrono::steady_clock::now();
  client.stop();
  auto t_end = std::chrono::steady_clock::now();

  auto elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(t_end - t_start).count();
  check(elapsed_ms < 50, "Second TdClient::stop is idempotent and fast (< 50ms)");
  std::cout << "[PASS] test_td_client_double_stop_idempotency\n";
}

int main() {
  std::cout << "Running TdClient flush and lifecycle unit tests...\n";
  test_td_client_flush_timing();
  test_td_client_double_stop_idempotency();
  std::cout << "All TdClient flush unit tests passed successfully.\n";
  return 0;
}
