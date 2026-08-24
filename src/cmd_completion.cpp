#include "grm/app.hpp"
#include "grm/command_registry.hpp"
#include "grm/logger.hpp"
#include <expected>
#include <iostream>
#include <string>
#include <vector>

namespace grm {

CommandSpec get_completion_spec() {
  return CommandSpec{
      "completion",
      "Generate shell autocompletion script for bash, zsh, or fish",
      {SubcommandSpec{
           "bash",
           "",
           "Output context-aware Bash autocompletion script",
           {OptionSpec{
               "-h", "--help", "", "Show completion bash help message", {}}}},
       SubcommandSpec{
           "zsh",
           "",
           "Output Zsh autocompletion function script",
           {OptionSpec{
               "-h", "--help", "", "Show completion zsh help message", {}}}},
       SubcommandSpec{
           "fish",
           "",
           "Output Fish autocompletion definitions",
           {OptionSpec{
               "-h", "--help", "", "Show completion fish help message", {}}}}},
      {}};
}

std::expected<int, std::string>
App::cmd_completion(const std::vector<std::string> &args) {
  if (args.empty() || (args.size() == 1 && is_help_requested(args))) {
    print_command_help("completion");
    return 0;
  }

  const std::string &shell = args[0];
  std::vector<std::string> sub_opts(args.begin() + 1, args.end());

  if (shell != "bash" && shell != "zsh" && shell != "fish") {
    print_command_help("completion");
    return std::unexpected("Unsupported shell: " + shell +
                           "\nSupported shells: bash, zsh, fish");
  }

  if (is_help_requested(sub_opts)) {
    print_subcommand_help("completion", shell);
    return 0;
  }

  std::cout << CommandRegistry::get_instance().render_completion(shell);
  return 0;
}

} // namespace grm
