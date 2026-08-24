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

  // Test 6b: Msg unpin --help returns 0
  auto res6b = app.run({"msg", "unpin", "--help"});
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
  assert(res11c.error().find("Unsupported shell: invalid_shell") !=
         std::string::npos);

  // Test 12: Logout command with --help returns 0
  auto res12a = app.run({"logout", "--help"});
  assert(res12a.has_value());
  assert(*res12a == 0);

  auto res12b = app.run({"logout", "-h"});
  assert(res12b.has_value());
  assert(*res12b == 0);

  // Test 13: Story commands routing and help
  auto res13a = app.run({"story"});
  assert(res13a.has_value());
  assert(*res13a == 0);

  auto res13b = app.run({"story", "--help"});
  assert(res13b.has_value());
  assert(*res13b == 0);

  const std::vector<std::string> story_subcmds = {
      "post", "edit",  "ls",    "delete",  "info",   "viewers",
      "pin",  "unpin", "react", "privacy", "stealth"};
  for (const auto &sub : story_subcmds) {
    auto res_help = app.run({"story", sub, "--help"});
    assert(res_help.has_value());
    assert(*res_help == 0);
  }

  auto res13c = app.run({"story", "invalid_story_subcmd"});
  assert(!res13c.has_value());
  assert(
      res13c.error().find("Unknown story subcommand: invalid_story_subcmd") !=
      std::string::npos);

  // Test 14: Status commands routing and help
  auto res14a = app.run({"status"});
  assert(res14a.has_value());
  assert(*res14a == 0);

  auto res14b = app.run({"status", "--help"});
  assert(res14b.has_value());
  assert(*res14b == 0);

  const std::vector<std::string> status_subcmds = {"ls", "set", "clear"};
  for (const auto &sub : status_subcmds) {
    auto res_help = app.run({"status", sub, "--help"});
    assert(res_help.has_value());
    assert(*res_help == 0);
  }

  // Test 15: Chat, Folder, Search subcommand help checks
  const std::vector<std::string> chat_subcmds = {
      "ls",       "create", "info",  "set-title",
      "set-desc", "pin",    "unpin", "delete"};
  for (const auto &sub : chat_subcmds) {
    auto res_help = app.run({"chat", sub, "--help"});
    assert(res_help.has_value());
    assert(*res_help == 0);
  }

  const std::vector<std::string> folder_subcmds = {"ls", "create", "edit",
                                                   "delete"};
  for (const auto &sub : folder_subcmds) {
    auto res_help = app.run({"folder", sub, "--help"});
    assert(res_help.has_value());
    assert(*res_help == 0);
  }

  const std::vector<std::string> search_subcmds = {
      "chats",      "supergroups", "channels", "msgs",     "users",   "files",
      "supergroup", "groups",      "channel",  "messages", "contacts"};
  for (const auto &sub : search_subcmds) {
    auto res_help = app.run({"search", sub, "--help"});
    assert(res_help.has_value());
    assert(*res_help == 0);
  }

  const std::vector<std::string> completion_subcmds = {"bash", "zsh", "fish"};
  for (const auto &sub : completion_subcmds) {
    auto res_help = app.run({"completion", sub, "--help"});
    assert(res_help.has_value());
    assert(*res_help == 0);
  }

  // Test 16: Stdout content verification for subcommand help isolation
  {
    std::stringstream buffer;
    std::streambuf *old = std::cout.rdbuf(buffer.rdbuf());
    auto res_story_ls = app.run({"story", "ls", "--help"});
    std::cout.rdbuf(old);
    assert(res_story_ls.has_value() && *res_story_ls == 0);
    std::string out = buffer.str();
    assert(out.find("Usage: grm story ls") != std::string::npos);
    assert(out.find("--pinned") != std::string::npos);
    assert(out.find("grm story post") == std::string::npos);
  }

  std::cout << "test_cli_routing passed successfully!\n";
  return 0;
}
