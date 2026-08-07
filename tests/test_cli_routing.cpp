#include "grm/app.hpp"
#include <cassert>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

int main() {
  std::cout << "Running test_cli_routing...\n";

  grm::Config cfg;
  grm::App app(cfg);

  // Test 1: Empty command returns 0 and prints top-level usage
  auto res1 = app.run({});
  assert(res1.has_value());
  assert(*res1 == 0);

  // Test 2: Global help flag returns 0
  auto res2 = app.run({"--help"});
  assert(res2.has_value());
  assert(*res2 == 0);

  // Test 2b: Global version flag returns 0
  auto res2b = app.run({"--version"});
  assert(res2b.has_value());
  assert(*res2b == 0);

  auto res2c = app.run({"-V"});
  assert(res2c.has_value());
  assert(*res2c == 0);

  // Test 3: Chat command without subcommand returns 0 (prints chat help)
  auto res3 = app.run({"chat"});
  assert(res3.has_value());
  assert(*res3 == 0);

  // Test 4: Chat command with --help returns 0
  auto res4 = app.run({"chat", "--help"});
  assert(res4.has_value());
  assert(*res4 == 0);

  // Test 5: Chat command with invalid subcommand returns unexpected error
  auto res5 = app.run({"chat", "invalid_subcmd"});
  assert(!res5.has_value());
  assert(res5.error().find("Unknown chat subcommand: invalid_subcmd") !=
         std::string::npos);

  // Test 6: Msg command with invalid subcommand returns unexpected error
  auto res6 = app.run({"msg", "invalid_subcmd"});
  assert(!res6.has_value());
  assert(res6.error().find("Unknown msg subcommand: invalid_subcmd") !=
         std::string::npos);

  // Test 6b: Msg unpin --all returns 0
  auto res6b = app.run({"msg", "unpin", "--all", "12345"});
  assert(res6b.has_value());
  assert(*res6b == 0);

  // Test 7: Topic command with invalid subcommand returns unexpected error
  auto res7 = app.run({"topic", "invalid_subcmd"});
  assert(!res7.has_value());
  assert(res7.error().find("Unknown topic subcommand: invalid_subcmd") !=
         std::string::npos);

  // Test 8: File command with invalid subcommand returns unexpected error
  auto res8 = app.run({"file", "invalid_subcmd"});
  assert(!res8.has_value());
  assert(res8.error().find("Unknown file subcommand: invalid_subcmd") !=
         std::string::npos);

  // Test 9: Unknown top-level command returns unexpected error
  auto res9 = app.run({"unknown_command"});
  assert(!res9.has_value());
  assert(res9.error().find("Unknown command: unknown_command") !=
         std::string::npos);

  // Test 10: -H / --help=all master help returns 0
  auto res10a = app.run({"-H"});
  assert(res10a.has_value());
  assert(*res10a == 0);

  auto res10b = app.run({"--help=all"});
  assert(res10b.has_value());
  assert(*res10b == 0);

  // Test 11: Gibberish command strings return unexpected error
  auto res11a = app.run({"12345_gibberish_#$%^"});
  assert(!res11a.has_value());
  assert(res11a.error().find("Unknown command") != std::string::npos);

  auto res11b = app.run({"chat", "gibberish_action"});
  assert(!res11b.has_value());
  assert(res11b.error().find("Unknown chat subcommand") != std::string::npos);

  auto res11c = app.run({"completion", "invalid_shell"});
  assert(!res11c.has_value());
  assert(res11c.error().find("Unknown shell: invalid_shell") != std::string::npos);

  std::cout << "test_cli_routing passed successfully!\n";
  return 0;
}
