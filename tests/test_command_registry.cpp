#include "grm/command_registry.hpp"
#include <cassert>
#include <iostream>
#include <string>

int main() {
  const auto &registry = grm::CommandRegistry::get_instance();

  // Test 1: Global help screen rendering
  std::string global_help = registry.render_global_help();
  assert(!global_help.empty());
  assert(global_help.find("grm - Group & Telegram Manager CLI") != std::string::npos);
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
  assert(bash_comp.find("photo video doc audio all") != std::string::npos);

  // Test 5: Zsh completion script rendering
  std::string zsh_comp = registry.render_completion("zsh");
  assert(!zsh_comp.empty());
  assert(zsh_comp.find("#compdef grm") != std::string::npos);

  // Test 6: Fish completion script rendering
  std::string fish_comp = registry.render_completion("fish");
  assert(!fish_comp.empty());
  assert(fish_comp.find("complete -c grm") != std::string::npos);

  std::cout << "All CommandRegistry unit tests passed successfully!" << std::endl;
  return 0;
}
