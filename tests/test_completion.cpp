#include "grm/app.hpp"
#include <cassert>
#include <iostream>
#include <string>

int main() {
  grm::Config config;
  grm::CliOptions options;
  grm::App app(config, options);

  // Test 1: grm completion without args returns unexpected error
  auto res1 = app.run({"completion"});
  assert(!res1.has_value());
  assert(res1.error().find("Usage: grm completion <shell>") !=
         std::string::npos);

  // Test 2: grm completion with invalid shell returns unexpected error
  auto res2 = app.run({"completion", "invalid_shell"});
  assert(!res2.has_value());
  assert(res2.error().find("Unsupported shell: invalid_shell") !=
         std::string::npos);

  // Test 3: grm completion bash returns 0
  auto res3 = app.run({"completion", "bash"});
  assert(res3.has_value());
  assert(*res3 == 0);

  // Test 4: grm completion zsh returns 0
  auto res4 = app.run({"completion", "zsh"});
  assert(res4.has_value());
  assert(*res4 == 0);

  // Test 5: grm completion fish returns 0
  auto res5 = app.run({"completion", "fish"});
  assert(res5.has_value());
  assert(*res5 == 0);

  std::cout << "All completion CLI tests passed successfully!" << std::endl;
  return 0;
}
