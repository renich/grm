#include "grm/command_registry.hpp"
#include <cassert>
#include <iostream>
#include <string>

int main() {
  const auto &registry = grm::CommandRegistry::get_instance();

  // Test 1: Global help screen rendering
  std::string global_help = registry.render_global_help();
  assert(!global_help.empty());
  assert(global_help.find("grm: Telegram CLI client") != std::string::npos);
  assert(global_help.find("Commands:") != std::string::npos);

  // Test 2: Subcommand help screen rendering for 'file'
  std::string file_help = registry.render_command_help("file");
  assert(!file_help.empty());
  assert(file_help.find("grm file get") != std::string::npos);
  // Test 2b: Subcommand help screen rendering for 'login'
  std::string login_help = registry.render_command_help("login");
  assert(!login_help.empty());
  assert(login_help.find("--phone") != std::string::npos);
  assert(login_help.find("--qr") != std::string::npos);

  // Test 3: Command overview help screen rendering for 'msg'
  std::string msg_help = registry.render_command_help("msg");
  assert(!msg_help.empty());
  assert(msg_help.find("grm msg unpin") != std::string::npos);
  assert(msg_help.find("Run 'grm msg <subcommand> --help'") !=
         std::string::npos);

  // Test 3a: Dedicated subcommand help screen rendering for 'msg unpin'
  std::string msg_unpin_help = registry.render_subcommand_help("msg", "unpin");
  assert(!msg_unpin_help.empty());
  assert(msg_unpin_help.find("Usage: grm msg unpin") != std::string::npos);
  assert(msg_unpin_help.find("--all") != std::string::npos);

  // Test 3b: Dedicated subcommand help screen rendering for 'story ls'
  std::string story_ls_help = registry.render_subcommand_help("story", "ls");
  assert(!story_ls_help.empty());
  assert(story_ls_help.find("Usage: grm story ls") != std::string::npos);
  assert(story_ls_help.find("--pinned") != std::string::npos);
  assert(story_ls_help.find("--all") != std::string::npos);
  assert(story_ls_help.find("--chat") != std::string::npos);

  // Test 3c: Dedicated subcommand help screen rendering for 'status set'
  std::string status_set_help =
      registry.render_subcommand_help("status", "set");
  assert(!status_set_help.empty());
  assert(status_set_help.find("Usage: grm status set") != std::string::npos);
  assert(status_set_help.find("--emoji") != std::string::npos);
  assert(status_set_help.find("--duration") != std::string::npos);

  // Test 3d: Subcommand help screen rendering for 'logout'
  std::string logout_help = registry.render_command_help("logout");
  assert(!logout_help.empty());
  assert(logout_help.find("grm logout") != std::string::npos);
  assert(logout_help.find("Log out from Telegram") != std::string::npos);

  // Test 4: Bash completion script rendering
  std::string bash_comp = registry.render_completion("bash");
  assert(!bash_comp.empty());
  assert(bash_comp.find("_grm_completions()") != std::string::npos);
  assert(bash_comp.find("--type") != std::string::npos);

  // Test 5: Zsh completion script rendering
  std::string zsh_comp = registry.render_completion("zsh");
  assert(!zsh_comp.empty());
  assert(zsh_comp.find("#compdef grm") != std::string::npos);

  // Test 7: JSON global help rendering (compact vs pretty)
  std::string compact_json = registry.render_global_help_json(false);
  assert(!compact_json.empty());
  assert(compact_json.find("\"program\":\"grm\"") != std::string::npos);
  assert(compact_json.find("\"global_options\"") != std::string::npos);
  // Compact output must not contain pretty-print indentation newlines before
  // keys
  assert(compact_json.find("{\n  \"program\"") == std::string::npos);

  std::string pretty_json = registry.render_global_help_json(true);
  assert(!pretty_json.empty());
  assert(pretty_json.find("{\n  \"program\": \"grm\"") != std::string::npos);

  // Test 8: JSON command help rendering for 'chat'
  std::string chat_json = registry.render_command_help_json("chat", false);
  assert(!chat_json.empty());
  assert(chat_json.find("\"command\":\"chat\"") != std::string::npos);
  assert(chat_json.find("\"subcommands\"") != std::string::npos);

  // Test 8b: JSON subcommand help rendering for 'story post'
  std::string story_post_json =
      registry.render_subcommand_help_json("story", "post", false);
  assert(!story_post_json.empty());
  assert(story_post_json.find("\"command\":\"story\"") != std::string::npos);
  assert(story_post_json.find("\"subcommand\":\"post\"") != std::string::npos);
  assert(story_post_json.find("\"privacy\"") != std::string::npos);

  // Test 9: JSON all master help rendering (-H / --help=all)
  std::string all_json_compact = registry.render_all_help_json(false);
  assert(!all_json_compact.empty());
  assert(all_json_compact.find("\"program\":\"grm\"") != std::string::npos);
  assert(all_json_compact.find("\"commands\"") != std::string::npos);

  std::string all_json_pretty = registry.render_all_help_json(true);
  assert(!all_json_pretty.empty());
  assert(all_json_pretty.find("{\n  \"program\": \"grm\"") !=
         std::string::npos);

  std::cout << "All CommandRegistry unit tests passed successfully!"
            << std::endl;
  return 0;
}
