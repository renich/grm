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
  assert(file_help.find("--type") != std::string::npos);

  // Test 3: Subcommand help screen rendering for 'msg'
  std::string msg_help = registry.render_command_help("msg");
  assert(!msg_help.empty());
  assert(msg_help.find("grm msg unpin") != std::string::npos);
  assert(msg_help.find("--all") != std::string::npos);

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
  // Compact output must not contain pretty-print indentation newlines before keys
  assert(compact_json.find("{\n  \"program\"") == std::string::npos);

  std::string pretty_json = registry.render_global_help_json(true);
  assert(!pretty_json.empty());
  assert(pretty_json.find("{\n  \"program\": \"grm\"") != std::string::npos);

  // Test 8: JSON command help rendering for 'chat'
  std::string chat_json = registry.render_command_help_json("chat", false);
  assert(!chat_json.empty());
  assert(chat_json.find("\"command\":\"chat\"") != std::string::npos);
  assert(chat_json.find("\"subcommands\"") != std::string::npos);

  // Test 9: JSON all master help rendering (-H / --help=all)
  std::string all_json_compact = registry.render_all_help_json(false);
  assert(!all_json_compact.empty());
  assert(all_json_compact.find("\"program\":\"grm\"") != std::string::npos);
  assert(all_json_compact.find("\"commands\"") != std::string::npos);

  std::string all_json_pretty = registry.render_all_help_json(true);
  assert(!all_json_pretty.empty());
  assert(all_json_pretty.find("{\n  \"program\": \"grm\"") != std::string::npos);

  std::cout << "All CommandRegistry unit tests passed successfully!" << std::endl;
  return 0;
}
