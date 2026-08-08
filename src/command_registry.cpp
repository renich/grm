#include "grm/command_registry.hpp"
#include "grm/app.hpp"
#include <algorithm>
#include <format>
#include <iostream>
#include <json-c/json.h>
#include <sstream>

namespace grm {

CommandRegistry::CommandRegistry() {
  commands_.push_back(get_login_spec());
  commands_.push_back(get_chat_spec());
  commands_.push_back(get_msg_spec());
  commands_.push_back(get_topic_spec());
  commands_.push_back(get_file_spec());
  commands_.push_back(get_folder_spec());
  commands_.push_back(App::get_search_spec());
  commands_.push_back(get_completion_spec());
}

const CommandRegistry &CommandRegistry::get_instance() {
  static const CommandRegistry instance;
  return instance;
}

std::string CommandRegistry::render_global_help() const {
  std::stringstream ss;
  ss << "grm: Telegram CLI client v0.7.0\n\n"
     << "Usage: grm [OPTION]... [COMMAND] [ARGS]...\n\n"
     << "Global Options:\n"
     << "  -h, --help            Show summary help screen\n"
     << "  -H, --help=all        Show exhaustive help for all commands and subcommands\n"
     << "  -V, --version         Display version and build environment info\n"
     << "  -v, --verbose         Enable verbose TDLib state output\n"
     << "  -d, --debug           Enable debug tracing\n"
     << "  -q, --quiet           Suppress non-error informational messages\n"
     << "  -c, --config <file>   Path to custom configuration file\n"
     << "  -T, --test-dc         Connect to Telegram Test Data Center environment\n"
     << "  -F, --format <fmt>    Output format: human, markdown, json, plain (default: auto)\n"
     << "      --color <mode>    Set ANSI color mode: auto, always, never (or --no-color)\n\n"
     << "Commands:\n";

  for (const auto &cmd : commands_) {
    ss << std::format("  {:<21} {}\n", cmd.name, cmd.description);
  }

  ss << "\nRun 'grm <command> --help' or 'grm --help=all' for details.\n";
  return ss.str();
}

std::string CommandRegistry::render_command_help(const std::string &cmd_name) const {
  auto it = std::ranges::find_if(commands_, [&](const CommandSpec &c) {
    return c.name == cmd_name;
  });

  if (it == commands_.end()) {
    return render_global_help();
  }

  std::stringstream ss;
  ss << std::format("Usage: grm {} <subcommand> [options] [args]\n\n{}\n\nSubcommands:\n", it->name, it->description);

  for (const auto &sub : it->subcommands) {
    std::string line = std::format("grm {} {} {}", it->name, sub.name, sub.synopsis);
    ss << std::format("  {:<64} {}\n", line, sub.description);
  }

  ss << "\nOptions:\n";
  for (const auto &sub : it->subcommands) {
    for (const auto &opt : sub.options) {
      std::string flags;
      if (!opt.short_flag.empty() && !opt.long_flag.empty()) {
        flags = std::format("{}, {}", opt.short_flag, opt.long_flag);
      } else if (!opt.short_flag.empty()) {
        flags = opt.short_flag;
      } else {
        flags = opt.long_flag;
      }

      if (!opt.value_hint.empty()) {
        flags += " " + opt.value_hint;
      }

      ss << std::format("  {:<31} {}\n", flags, opt.description);
    }
  }

  return ss.str();
}

std::string CommandRegistry::render_all_help() const {
  std::stringstream ss;
  ss << "grm: Telegram CLI client v0.7.0\n\n"
     << "Usage: grm [OPTION]... [COMMAND] [ARGS]...\n\n"
     << "Global Options:\n"
     << "  -h, --help            Show summary help screen\n"
     << "  -H, --help=all        Show exhaustive help for all commands and subcommands\n"
     << "  -V, --version         Display version and build environment info\n"
     << "  -v, --verbose         Enable verbose TDLib state output\n"
     << "  -d, --debug           Enable debug tracing\n"
     << "  -q, --quiet           Suppress non-error informational messages\n"
     << "  -c, --config <file>   Path to custom configuration file\n"
     << "  -T, --test-dc         Connect to Telegram Test Data Center environment\n"
     << "  -F, --format <fmt>    Output format: human, markdown, json, plain (default: auto)\n"
     << "      --color <mode>    Set ANSI color mode: auto, always, never (or --no-color)\n\n"
     << "Commands:\n\n";

  for (size_t c_idx = 0; c_idx < commands_.size(); ++c_idx) {
    const auto &cmd = commands_[c_idx];
    ss << std::format("{}: {}\n", cmd.name, cmd.description);

    for (size_t s_idx = 0; s_idx < cmd.subcommands.size(); ++s_idx) {
      const auto &sub = cmd.subcommands[s_idx];
      std::string line;
      if (sub.synopsis.empty()) {
        line = std::format("grm {} {}", cmd.name, sub.name);
      } else {
        line = std::format("grm {} {} {}", cmd.name, sub.name, sub.synopsis);
      }

      ss << std::format("  {}\n", line);
      for (const auto &opt : sub.options) {
        std::string flags;
        if (!opt.short_flag.empty() && !opt.long_flag.empty()) {
          flags = std::format("{}, {}", opt.short_flag, opt.long_flag);
        } else if (!opt.short_flag.empty()) {
          flags = opt.short_flag;
        } else {
          flags = opt.long_flag;
        }

        if (!opt.value_hint.empty()) {
          flags += " " + opt.value_hint;
        }

        std::string opt_desc = opt.description;
        if (!opt.choices.empty()) {
          std::string choices_str;
          for (size_t i = 0; i < opt.choices.size(); ++i) {
            if (i > 0) choices_str += "|";
            choices_str += opt.choices[i];
          }
          opt_desc += " (" + choices_str + ")";
        }

        ss << std::format("    {:<32} {}\n", flags, opt_desc);
      }

      ss << "\n";
    }

    if (c_idx + 1 < commands_.size()) {
      ss << "\n";
    }
  }

  return ss.str();
}

std::string CommandRegistry::render_global_help_json(bool pretty) const {
  json_object *root = json_object_new_object();
  json_object_object_add(root, "program", json_object_new_string("grm"));
  json_object_object_add(root, "description", json_object_new_string("Telegram CLI client"));
  json_object_object_add(root, "version", json_object_new_string("0.7.0"));

  json_object *global_opts_arr = json_object_new_array();
  const std::vector<OptionSpec> global_opts = {
      {"-h", "--help", "", "Show summary help screen", {}},
      {"-H", "--help=all", "", "Show exhaustive help for all commands and subcommands", {}},
      {"-V", "--version", "", "Display version and build environment info", {}},
      {"-v", "--verbose", "", "Enable verbose TDLib state output", {}},
      {"-d", "--debug", "", "Enable debug tracing", {}},
      {"-q", "--quiet", "", "Suppress non-error informational messages", {}},
      {"-c", "--config", "<file>", "Path to custom configuration file", {}},
      {"-T", "--test-dc", "", "Connect to Telegram Test Data Center environment", {}},
      {"-F", "--format", "<fmt>", "Output format: human, markdown, json, plain (default: auto)", {"human", "markdown", "json", "plain"}},
      {"-p", "--pretty", "", "Pretty-print JSON output formatting (with -F json)", {}},
      {"", "--color", "<mode>", "Set ANSI color mode: auto, always, never (or --no-color)", {"auto", "always", "never"}}
  };

  for (const auto &opt : global_opts) {
    json_object *o = json_object_new_object();
    json_object_object_add(o, "short_flag", json_object_new_string(opt.short_flag.c_str()));
    json_object_object_add(o, "long_flag", json_object_new_string(opt.long_flag.c_str()));
    json_object_object_add(o, "value_hint", json_object_new_string(opt.value_hint.c_str()));
    json_object_object_add(o, "description", json_object_new_string(opt.description.c_str()));
    json_object_array_add(global_opts_arr, o);
  }
  json_object_object_add(root, "global_options", global_opts_arr);

  json_object *cmds_arr = json_object_new_array();
  for (const auto &cmd : commands_) {
    json_object *c = json_object_new_object();
    json_object_object_add(c, "name", json_object_new_string(cmd.name.c_str()));
    json_object_object_add(c, "description", json_object_new_string(cmd.description.c_str()));
    json_object_array_add(cmds_arr, c);
  }
  json_object_object_add(root, "commands", cmds_arr);

  int flags = pretty ? (JSON_C_TO_STRING_PRETTY | JSON_C_TO_STRING_SPACED) : JSON_C_TO_STRING_PLAIN;
  std::string result = json_object_to_json_string_ext(root, flags);
  json_object_put(root);
  return result + "\n";
}

std::string CommandRegistry::render_command_help_json(const std::string &cmd_name, bool pretty) const {
  auto it = std::ranges::find_if(commands_, [&](const CommandSpec &c) {
    return c.name == cmd_name;
  });

  if (it == commands_.end()) {
    return render_global_help_json(pretty);
  }

  json_object *root = json_object_new_object();
  json_object_object_add(root, "program", json_object_new_string("grm"));
  json_object_object_add(root, "command", json_object_new_string(it->name.c_str()));
  json_object_object_add(root, "description", json_object_new_string(it->description.c_str()));

  json_object *subs_arr = json_object_new_array();
  for (const auto &sub : it->subcommands) {
    json_object *s = json_object_new_object();
    json_object_object_add(s, "name", json_object_new_string(sub.name.c_str()));
    json_object_object_add(s, "synopsis", json_object_new_string(sub.synopsis.c_str()));
    json_object_object_add(s, "description", json_object_new_string(sub.description.c_str()));

    json_object *opts_arr = json_object_new_array();
    for (const auto &opt : sub.options) {
      json_object *o = json_object_new_object();
      json_object_object_add(o, "short_flag", json_object_new_string(opt.short_flag.c_str()));
      json_object_object_add(o, "long_flag", json_object_new_string(opt.long_flag.c_str()));
      json_object_object_add(o, "value_hint", json_object_new_string(opt.value_hint.c_str()));
      json_object_object_add(o, "description", json_object_new_string(opt.description.c_str()));

      json_object *choices_arr = json_object_new_array();
      for (const auto &choice : opt.choices) {
        json_object_array_add(choices_arr, json_object_new_string(choice.c_str()));
      }
      json_object_object_add(o, "choices", choices_arr);

      json_object_array_add(opts_arr, o);
    }
    json_object_object_add(s, "options", opts_arr);
    json_object_array_add(subs_arr, s);
  }
  json_object_object_add(root, "subcommands", subs_arr);

  int flags = pretty ? (JSON_C_TO_STRING_PRETTY | JSON_C_TO_STRING_SPACED) : JSON_C_TO_STRING_PLAIN;
  std::string result = json_object_to_json_string_ext(root, flags);
  json_object_put(root);
  return result + "\n";
}

std::string CommandRegistry::render_all_help_json(bool pretty) const {
  json_object *root = json_object_new_object();
  json_object_object_add(root, "program", json_object_new_string("grm"));
  json_object_object_add(root, "description", json_object_new_string("Telegram CLI client"));
  json_object_object_add(root, "version", json_object_new_string("0.7.0"));

  json_object *global_opts_arr = json_object_new_array();
  const std::vector<OptionSpec> global_opts = {
      {"-h", "--help", "", "Show summary help screen", {}},
      {"-H", "--help=all", "", "Show exhaustive help for all commands and subcommands", {}},
      {"-V", "--version", "", "Display version and build environment info", {}},
      {"-v", "--verbose", "", "Enable verbose TDLib state output", {}},
      {"-d", "--debug", "", "Enable debug tracing", {}},
      {"-q", "--quiet", "", "Suppress non-error informational messages", {}},
      {"-c", "--config", "<file>", "Path to custom configuration file", {}},
      {"-T", "--test-dc", "", "Connect to Telegram Test Data Center environment", {}},
      {"-F", "--format", "<fmt>", "Output format: human, markdown, json, plain (default: auto)", {"human", "markdown", "json", "plain"}},
      {"-p", "--pretty", "", "Pretty-print JSON output formatting (with -F json)", {}},
      {"", "--color", "<mode>", "Set ANSI color mode: auto, always, never (or --no-color)", {"auto", "always", "never"}}
  };

  for (const auto &opt : global_opts) {
    json_object *o = json_object_new_object();
    json_object_object_add(o, "short_flag", json_object_new_string(opt.short_flag.c_str()));
    json_object_object_add(o, "long_flag", json_object_new_string(opt.long_flag.c_str()));
    json_object_object_add(o, "value_hint", json_object_new_string(opt.value_hint.c_str()));
    json_object_object_add(o, "description", json_object_new_string(opt.description.c_str()));
    json_object_array_add(global_opts_arr, o);
  }
  json_object_object_add(root, "global_options", global_opts_arr);

  json_object *cmds_arr = json_object_new_array();
  for (const auto &cmd : commands_) {
    json_object *c = json_object_new_object();
    json_object_object_add(c, "name", json_object_new_string(cmd.name.c_str()));
    json_object_object_add(c, "description", json_object_new_string(cmd.description.c_str()));

    json_object *subs_arr = json_object_new_array();
    for (const auto &sub : cmd.subcommands) {
      json_object *s = json_object_new_object();
      json_object_object_add(s, "name", json_object_new_string(sub.name.c_str()));
      json_object_object_add(s, "synopsis", json_object_new_string(sub.synopsis.c_str()));
      json_object_object_add(s, "description", json_object_new_string(sub.description.c_str()));

      json_object *opts_arr = json_object_new_array();
      for (const auto &opt : sub.options) {
        json_object *o = json_object_new_object();
        json_object_object_add(o, "short_flag", json_object_new_string(opt.short_flag.c_str()));
        json_object_object_add(o, "long_flag", json_object_new_string(opt.long_flag.c_str()));
        json_object_object_add(o, "value_hint", json_object_new_string(opt.value_hint.c_str()));
        json_object_object_add(o, "description", json_object_new_string(opt.description.c_str()));

        json_object *choices_arr = json_object_new_array();
        for (const auto &choice : opt.choices) {
          json_object_array_add(choices_arr, json_object_new_string(choice.c_str()));
        }
        json_object_object_add(o, "choices", choices_arr);

        json_object_array_add(opts_arr, o);
      }
      json_object_object_add(s, "options", opts_arr);
      json_object_array_add(subs_arr, s);
    }
    json_object_object_add(c, "subcommands", subs_arr);
    json_object_array_add(cmds_arr, c);
  }
  json_object_object_add(root, "commands", cmds_arr);

  int flags = pretty ? (JSON_C_TO_STRING_PRETTY | JSON_C_TO_STRING_SPACED) : JSON_C_TO_STRING_PLAIN;
  std::string result = json_object_to_json_string_ext(root, flags);
  json_object_put(root);
  return result + "\n";
}

std::string CommandRegistry::render_completion(const std::string &shell) const {

  if (shell == "bash") {
    std::stringstream ss;
    ss << "#!/usr/bin/bash\n"
       << "# Bash completion script for grm (Group & Telegram Manager CLI)\n"
       << "# Generated dynamically by introspection of CommandRegistry\n\n"
       << "_grm_completions() {\n"
       << "  local cur prev words cword\n"
       << "  _init_completion || return\n\n";

    std::string global_opts =
        "-h --help --help=all -V --version -v --verbose -d --debug -q --quiet "
        "-c --config -T --test-dc -F --format --color --no-color";

    std::string cmd_list_str;
    for (size_t i = 0; i < commands_.size(); ++i) {
      if (i > 0) {
        cmd_list_str += " ";
      }
      cmd_list_str += commands_[i].name;
    }

    ss << "  local global_opts=\"" << global_opts << "\"\n";
    ss << "  local commands=\"" << cmd_list_str << "\"\n\n";

    ss << R"(  if [[ ${cword} -eq 1 ]]; then
    if [[ "${cur}" == -* ]]; then
      COMPREPLY=($(compgen -W "${global_opts}" -- "${cur}"))
    else
      COMPREPLY=($(compgen -W "${commands}" -- "${cur}"))
    fi
    return 0
  fi

  local command="${words[1]}"

  case "${command}" in
)";

    for (const auto &cmd : commands_) {
      ss << "    " << cmd.name << ")\n";
      if (!cmd.subcommands.empty()) {
        std::string sub_names;
        for (size_t j = 0; j < cmd.subcommands.size(); ++j) {
          if (j > 0) {
            sub_names += " ";
          }
          sub_names += cmd.subcommands[j].name;
        }
        ss << "      if [[ ${cword} -eq 2 ]]; then\n";
        ss << "        if [[ \"${cur}\" == -* ]]; then\n";
        ss << "          COMPREPLY=($(compgen -W \"-h --help\" -- \"${cur}\"))\n";
        ss << "        else\n";
        ss << "          COMPREPLY=($(compgen -W \"" << sub_names << "\" -- \"${cur}\"))\n";
        ss << "        fi\n";

        for (const auto &sub : cmd.subcommands) {
          std::string flags = "-h --help";
          for (const auto &opt : sub.options) {
            if (!opt.short_flag.empty()) {
              flags += " " + opt.short_flag;
            }
            if (!opt.long_flag.empty()) {
              flags += " " + opt.long_flag;
            }
          }
          ss << "      elif [[ \"${words[2]}\" == \"" << sub.name << "\" ]]; then\n";
          ss << "        COMPREPLY=($(compgen -W \"" << flags << "\" -- \"${cur}\"))\n";
        }
        ss << "      fi\n";
      } else {
        ss << "      COMPREPLY=($(compgen -W \"-h --help\" -- \"${cur}\"))\n";
      }
      ss << "      ;;\n";
    }

    ss << R"(    *)
      ;;
  esac

  return 0
}

complete -F _grm_completions grm
)";
    return ss.str();
  }

  if (shell == "zsh") {
    std::stringstream ss;
    ss << "#compdef grm\n"
       << "# Zsh completion script for grm (Group & Telegram Manager CLI)\n"
       << "# Generated dynamically by introspection of CommandRegistry\n\n"
       << "_grm() {\n"
       << "  local -a commands\n"
       << "  commands=(\n";

    for (const auto &cmd : commands_) {
      std::string desc = cmd.description;
      size_t pos = 0;
      while ((pos = desc.find("'", pos)) != std::string::npos) {
        desc.replace(pos, 1, "'\\''");
        pos += 4;
      }
      ss << "    '" << cmd.name << ":" << desc << "'\n";
    }

    ss << R"(  )

  _arguments -s \
    '(-h --help)'{-h,--help}'[Show help screen]' \
    '--help=all[Show exhaustive master help]' \
    '(-V --version)'{-V,--version}'[Display version information]' \
    '(-v --verbose)'{-v,--verbose}'[Enable verbose TDLib log output]' \
    '(-d --debug)'{-d,--debug}'[Enable debug tracing]' \
    '(-q --quiet)'{-q,--quiet}'[Suppress non-error messages]' \
    '(-c --config)'{-c,--config}'[Custom config file path]:file:_files' \
    '(-T --test-dc)'{-T,--test-dc}'[Connect to Test DC]' \
    '(-F --format)'{-F,--format}'[Output format]:format:(human markdown json plain)' \
    '--color[Color mode]:mode:(auto always never)' \
    '1: :->command' \
    '*:: :->args'

  case $state in
    command)
      _describe -t commands 'grm command' commands
      ;;
    args)
      case $words[1] in
)";

    for (const auto &cmd : commands_) {
      if (!cmd.subcommands.empty()) {
        std::string sub_list;
        for (size_t j = 0; j < cmd.subcommands.size(); ++j) {
          if (j > 0) {
            sub_list += " ";
          }
          sub_list += cmd.subcommands[j].name;
        }
        ss << "        " << cmd.name << ")\n";
        ss << "          _arguments '1:subcommand:(" << sub_list << ")'\n";
        ss << "          ;;\n";
      }
    }

    ss << R"(      esac
      ;;
  esac
}

_grm "$@"
)";
    return ss.str();
  }

  if (shell == "fish") {
    std::stringstream ss;
    ss << "# Fish completion script for grm\n"
       << "# Generated dynamically by introspection of CommandRegistry\n";

    for (const auto &cmd : commands_) {
      ss << "complete -c grm -f -n '__fish_use_subcommand' -a '" << cmd.name
         << "' -d '" << cmd.description << "'\n";
      for (const auto &sub : cmd.subcommands) {
        ss << "complete -c grm -f -n '__fish_seen_subcommand_from " << cmd.name
           << "' -a '" << sub.name << "' -d '" << sub.description << "'\n";
      }
    }
    return ss.str();
  }

  return "";
}

} // namespace grm
