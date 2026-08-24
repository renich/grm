#pragma once

#include <string>
#include <vector>

namespace grm {

struct OptionSpec {
  std::string short_flag;
  std::string long_flag;
  std::string value_hint;
  std::string description;
  std::vector<std::string> choices;
};

struct SubcommandSpec {
  std::string name;
  std::string synopsis;
  std::string description;
  std::vector<OptionSpec> options;
};

struct CommandSpec {
  std::string name;
  std::string description;
  std::vector<SubcommandSpec> subcommands;
  std::vector<OptionSpec> global_options;
};

class CommandRegistry {
public:
  static const CommandRegistry &get_instance();

  [[nodiscard]] std::string render_global_help() const;
  [[nodiscard]] std::string
  render_command_help(const std::string &cmd_name) const;
  [[nodiscard]] std::string render_all_help() const;
  [[nodiscard]] std::string render_completion(const std::string &shell) const;

  [[nodiscard]] std::string render_global_help_json(bool pretty = false) const;
  [[nodiscard]] std::string
  render_command_help_json(const std::string &cmd_name,
                           bool pretty = false) const;
  [[nodiscard]] std::string render_all_help_json(bool pretty = false) const;

  [[nodiscard]] const std::vector<CommandSpec> &commands() const {
    return commands_;
  }

private:
  CommandRegistry();
  std::vector<CommandSpec> commands_;
};

} // namespace grm
