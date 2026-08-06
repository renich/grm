#include "grm/app.hpp"
#include "grm/command_registry.hpp"
#include "grm/logger.hpp"
#include <expected>
#include <iostream>
#include <string>
#include <vector>

namespace grm {

std::expected<int, std::string>
App::cmd_completion(const std::vector<std::string> &args) {
  if (args.empty()) {
    return std::unexpected(
        "Usage: grm completion <shell>\n\nSupported shells: bash, zsh, fish");
  }

  const std::string &shell = args[0];

  if (shell != "bash" && shell != "zsh" && shell != "fish") {
    return std::unexpected("Unsupported shell: " + shell +
                           "\nSupported shells: bash, zsh, fish");
  }

  std::cout << CommandRegistry::get_instance().render_completion(shell);
  return 0;
}

} // namespace grm
