#include "grm/app.hpp"
#include <cassert>
#include <iostream>
#include <string>

int main() {
  grm::Config config;
  grm::CliOptions options;
  grm::App app(config, options);

  // Test 1: grm completion without args returns 0 (prints overview help)
  auto res1 = app.run({"completion"});
  if (!res1.has_value() || *res1 != 0) {
    std::cerr << "Test 1 failed: expected 0 on completion overview"
              << std::endl;
    return 1;
  }

  // Test 2: grm completion with invalid shell returns unexpected error
  auto res2 = app.run({"completion", "invalid_shell"});
  if (res2.has_value()) {
    std::cerr << "Test 2 failed: expected error on invalid shell" << std::endl;
    return 1;
  }
  if (res2.error().find("Unsupported shell: invalid_shell") ==
      std::string::npos) {
    std::cerr << "Test 2 error message mismatch: " << res2.error() << std::endl;
    return 1;
  }

  // Test 3: grm completion bash returns 0
  auto res3 = app.run({"completion", "bash"});
  if (!res3.has_value() || *res3 != 0) {
    std::cerr << "Test 3 failed: expected 0 on completion bash" << std::endl;
    return 1;
  }

  // Test 4: grm completion zsh returns 0
  auto res4 = app.run({"completion", "zsh"});
  if (!res4.has_value() || *res4 != 0) {
    std::cerr << "Test 4 failed: expected 0 on completion zsh" << std::endl;
    return 1;
  }

  // Test 5: grm completion fish returns 0
  auto res5 = app.run({"completion", "fish"});
  if (!res5.has_value() || *res5 != 0) {
    std::cerr << "Test 5 failed: expected 0 on completion fish" << std::endl;
    return 1;
  }

  // Test 6: grm completion <shell> --help returns 0
  for (const auto &sh : {"bash", "zsh", "fish"}) {
    auto res_help = app.run({"completion", sh, "--help"});
    if (!res_help.has_value() || *res_help != 0) {
      std::cerr << "Test 6 failed for shell " << sh << std::endl;
      return 1;
    }
  }

  std::cout << "All completion CLI tests passed successfully!" << std::endl;
  return 0;
}
