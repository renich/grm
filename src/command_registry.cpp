#include "grm/command_registry.hpp"
#include <algorithm>
#include <format>
#include <iostream>
#include <sstream>

namespace grm {

CommandRegistry::CommandRegistry() {
  // Global Options
  std::vector<OptionSpec> global_opts = {
      {"-h", "--help", "", "Show command or subcommand help message and exit", {}},
      {"-V", "--version", "", "Display version and build environment information", {}},
      {"-v", "--verbose", "", "Enable verbose TDLib log output to stdout", {}},
      {"-d", "--debug", "", "Enable low-level debug tracing and JSON payload dumps", {}},
      {"-q", "--quiet", "", "Suppress non-error informational messages", {}},
      {"-c", "--config", "<file>", "Path to custom configuration file", {}},
      {"-T", "--test-dc", "", "Connect to Telegram Test Data Center environment", {}},
      {"-F", "--format", "<fmt>", "Set output format: human, markdown, json, plain (default: auto)", {"human", "markdown", "json", "plain"}},
      {"", "--color", "<mode>", "Set ANSI color mode: auto, always, never (or --no-color)", {"auto", "always", "never"}}
  };

  // 1. login
  commands_.push_back(CommandSpec{
      "login",
      "Authenticate Telegram account with TDLib",
      {
          SubcommandSpec{"login", "[-p|--phone <number>] [-k|--code <code>]", "Authenticate Telegram session interactively or non-interactively", {
              OptionSpec{"-p", "--phone", "<number>", "International phone number (e.g. +523330000000)", {}},
              OptionSpec{"-k", "--code", "<code>", "Authentication code received via Telegram or SMS", {}},
              OptionSpec{"-h", "--help", "", "Show login help message", {}}
          }}
      },
      global_opts
  });

  // 2. chat
  commands_.push_back(CommandSpec{
      "chat",
      "Manage Telegram chats, groups, and channels",
      {
          SubcommandSpec{"ls", "[-n|--limit <N>] [-S|--since <time>] [-f|--filter <pattern>]", "List active conversations, groups, channels, and private chats", {
              OptionSpec{"-n", "--limit", "<N>", "Maximum number of chats to display (default: 100)", {}},
              OptionSpec{"-S", "--since", "<time>", "Filter chats active since duration (e.g. '1 day ago')", {}},
              OptionSpec{"-f", "--filter", "<pattern>", "Filter chats by title or ID pattern filter", {}},
              OptionSpec{"-h", "--help", "", "Show list help message", {}}
          }},
          SubcommandSpec{"create", "<group|channel> [--private|--public] \"<title>\"", "Create a new basic group, supergroup, or broadcast channel", {
              OptionSpec{"", "--private", "", "Create as a private chat/channel", {}},
              OptionSpec{"", "--public", "", "Create as a public chat/channel", {}},
              OptionSpec{"-h", "--help", "", "Show create help message", {}}
          }},
          SubcommandSpec{"info", "<chat_id>", "Display detailed chat or supergroup metadata", {
              OptionSpec{"-h", "--help", "", "Show info help message", {}}
          }},
          SubcommandSpec{"set-title", "<chat_id> \"<title>\"", "Update group or channel title", {
              OptionSpec{"-h", "--help", "", "Show set-title help message", {}}
          }},
          SubcommandSpec{"set-desc", "<chat_id> \"<description>\"", "Update group or channel description", {
              OptionSpec{"-h", "--help", "", "Show set-desc help message", {}}
          }},
          SubcommandSpec{"pin", "<chat_id>", "Pin chat to top of chat list", {
              OptionSpec{"-h", "--help", "", "Show pin help message", {}}
          }},
          SubcommandSpec{"unpin", "<chat_id>", "Unpin chat from chat list", {
              OptionSpec{"-h", "--help", "", "Show unpin help message", {}}
          }},
          SubcommandSpec{"delete", "<chat_id>", "Delete chat history or leave group/channel", {
              OptionSpec{"-h", "--help", "", "Show delete help message", {}}
          }}
      },
      global_opts
  });

  // 3. msg
  commands_.push_back(CommandSpec{
      "msg",
      "Inspect, send, edit, search, pin, and export messages",
      {
          SubcommandSpec{"ls", "[-t|--topic <id>] [-n|--limit <N>] <chat_id>", "List recent messages from chat or topic thread", {
              OptionSpec{"-t", "--topic", "<id>", "Target specific forum topic thread ID", {}},
              OptionSpec{"-n", "--limit", "<N>", "Maximum messages to fetch (default: 20)", {}},
              OptionSpec{"-S", "--since", "<time>", "Filter messages since duration or ISO date", {}},
              OptionSpec{"-f", "--filter", "<pattern>", "Filter messages by regex pattern filter", {}},
              OptionSpec{"-r", "--reverse", "", "Display messages in reverse chronological order", {}},
              OptionSpec{"-h", "--help", "", "Show message list help message", {}}
          }},
          SubcommandSpec{"send", "[-a|--attach <file>] [-m|--media] [-C|--caption \"<text>\"] [-t|--topic <id>] <chat_id> [\"<message>\"]", "Send text message or file attachment(s)", {
              OptionSpec{"-a", "--attach", "<file>", "Attach file or document path to message", {}},
              OptionSpec{"-m", "--media", "", "Send attachment as inline visual media", {}},
              OptionSpec{"-C", "--caption", "<text>", "Set caption for file attachment", {}},
              OptionSpec{"-t", "--topic", "<id>", "Target specific forum topic thread ID", {}},
              OptionSpec{"-h", "--help", "", "Show send help message", {}}
          }},
          SubcommandSpec{"info", "<chat_id> <message_id>", "View message details and metadata", {
              OptionSpec{"-t", "--topic", "<id>", "Target specific forum topic thread ID", {}},
              OptionSpec{"-h", "--help", "", "Show info help message", {}}
          }},
          SubcommandSpec{"edit", "[-t|--topic <id>] <chat_id> <message_id> \"<new_text>\"", "Edit previously sent text message content", {
              OptionSpec{"-t", "--topic", "<id>", "Target specific forum topic thread ID", {}},
              OptionSpec{"-h", "--help", "", "Show edit help message", {}}
          }},
          SubcommandSpec{"search", "[-t|--topic <id>] [-q|--query \"<query>\"] [-n|--limit <N>] <chat_id>", "Search chat history using query pattern filter", {
              OptionSpec{"-q", "--query", "<query>", "Search query substring or regex pattern", {}},
              OptionSpec{"-t", "--topic", "<id>", "Target specific forum topic thread ID", {}},
              OptionSpec{"-n", "--limit", "<N>", "Maximum search results to return", {}},
              OptionSpec{"-h", "--help", "", "Show search help message", {}}
          }},
          SubcommandSpec{"export", "[-f|--format csv|json] [-o|--output <file>] [-t|--topic <id>] <chat_id>", "Export chat history to CSV or JSON file", {
              OptionSpec{"-f", "--format", "<fmt>", "Export format: csv or json (default: json)", {"csv", "json"}},
              OptionSpec{"-o", "--output", "<file>", "Destination export filepath", {}},
              OptionSpec{"-t", "--topic", "<id>", "Target specific forum topic thread ID", {}},
              OptionSpec{"-n", "--limit", "<N>", "Maximum messages to export", {}},
              OptionSpec{"-h", "--help", "", "Show export help message", {}}
          }},
          SubcommandSpec{"pin", "[-t|--topic <id>] <chat_id> <message_id>", "Pin message in chat or topic", {
              OptionSpec{"-t", "--topic", "<id>", "Target specific forum topic thread ID", {}},
              OptionSpec{"-h", "--help", "", "Show pin help message", {}}
          }},
          SubcommandSpec{"unpin", "[-a|--all] <chat_id> [<message_ids...>]", "Unpin message(s) or unpin all pinned messages in chat", {
              OptionSpec{"-a", "--all", "", "Unpin all pinned messages in chat", {}},
              OptionSpec{"-h", "--help", "", "Show unpin help message", {}}
          }},
          SubcommandSpec{"delete", "[-e|--for-everyone] <chat_id> <message_ids...>", "Delete message(s) from chat", {
              OptionSpec{"-e", "--for-everyone", "", "Delete message for all members", {}},
              OptionSpec{"-h", "--help", "", "Show delete help message", {}}
          }}
      },
      global_opts
  });

  // 4. topic
  commands_.push_back(CommandSpec{
      "topic",
      "Manage supergroup forum topics",
      {
          SubcommandSpec{"ls", "[-n|--limit <N>] <supergroup_id>", "List forum topics in supergroup", {
              OptionSpec{"-n", "--limit", "<N>", "Maximum topics to display (default: 100)", {}},
              OptionSpec{"-h", "--help", "", "Show topic list help message", {}}
          }},
          SubcommandSpec{"create", "[-e|--emoji <id>] <supergroup_id> \"<name>\"", "Create a new forum topic in supergroup", {
              OptionSpec{"-e", "--emoji", "<id>", "Custom Telegram emoji icon ID", {}},
              OptionSpec{"", "--icon-color", "<color>", "RGB icon color (e.g. 0x6FB9F0)", {}},
              OptionSpec{"-h", "--help", "", "Show create topic help message", {}}
          }},
          SubcommandSpec{"info", "<supergroup_id> <topic_id>", "View topic metadata and settings", {
              OptionSpec{"-h", "--help", "", "Show topic info help message", {}}
          }},
          SubcommandSpec{"edit", "[-e|--emoji <id>] <supergroup_id> <topic_id> [\"<name>\"]", "Edit topic title or icon", {
              OptionSpec{"-e", "--emoji", "<id>", "Custom Telegram emoji icon ID", {}},
              OptionSpec{"-h", "--help", "", "Show edit topic help message", {}}
          }},
          SubcommandSpec{"close", "<supergroup_id> <topic_id>", "Close forum topic thread", {
              OptionSpec{"-h", "--help", "", "Show close help message", {}}
          }},
          SubcommandSpec{"reopen", "<supergroup_id> <topic_id>", "Reopen closed forum topic thread", {
              OptionSpec{"-h", "--help", "", "Show reopen help message", {}}
          }},
          SubcommandSpec{"pin", "<supergroup_id> <topic_id>", "Pin topic to top of topic list", {
              OptionSpec{"-h", "--help", "", "Show pin help message", {}}
          }},
          SubcommandSpec{"unpin", "<supergroup_id> <topic_id>", "Unpin topic from topic list", {
              OptionSpec{"-h", "--help", "", "Show unpin help message", {}}
          }},
          SubcommandSpec{"delete", "<supergroup_id> <topic_id>", "Delete topic and message thread history", {
              OptionSpec{"-h", "--help", "", "Show delete topic help message", {}}
          }}
      },
      global_opts
  });

  // 5. file
  commands_.push_back(CommandSpec{
      "file",
      "Download attachments and media files",
      {
          SubcommandSpec{"get", "[-a|-A|--all] [-o|--output <dir|file>] [-t|--topic <id>] [-n|--limit <N>] [--type photo|video|doc|audio|all] <chat_id> [<message_ids...>]", "Download media and file attachments from chats and topics", {
              OptionSpec{"-a", "--all", "", "Bulk download all attachment files in chat or topic", {}},
              OptionSpec{"-A", "", "", "Bulk download all attachment files in chat or topic (alias for -a)", {}},
              OptionSpec{"-o", "--output", "<dir|file>", "Destination output directory or filepath", {}},
              OptionSpec{"-t", "--topic", "<id>", "Target specific forum topic thread ID", {}},
              OptionSpec{"-n", "--limit", "<N>", "Maximum messages to fetch or search", {}},
              OptionSpec{"", "--type", "<type>", "Filter media type for bulk download (photo, video, doc, audio, all)", {"photo", "video", "doc", "audio", "all"}},
              OptionSpec{"-h", "--help", "", "Show file get help message", {}}
          }}
      },
      global_opts
  });

  // 6. completion
  commands_.push_back(CommandSpec{
      "completion",
      "Generate shell autocompletion script for bash, zsh, or fish",
      {
          SubcommandSpec{"bash", "", "Output context-aware Bash autocompletion script", {}},
          SubcommandSpec{"zsh", "", "Output Zsh autocompletion function script", {}},
          SubcommandSpec{"fish", "", "Output Fish autocompletion definitions", {}}
      },
      global_opts
  });
}

const CommandRegistry &CommandRegistry::get_instance() {
  static const CommandRegistry instance;
  return instance;
}

std::string CommandRegistry::render_global_help() const {
  std::stringstream ss;
  ss << "grm - Group & Telegram Manager CLI (C++23 / TDLib)\n\n"
     << "Global Options:\n"
     << "  -h, --help            Show this help screen\n"
     << "  -V, --version         Display version and build info\n"
     << "  -v, --verbose         Enable verbose TDLib state output\n"
     << "  -d, --debug           Enable debug tracing\n"
     << "  -q, --quiet           Suppress informational messages\n"
     << "  -c, --config <file>   Custom configuration file path\n"
     << "  -T, --test-dc         Connect to Telegram Test Data Center environment\n"
     << "  -F, --format <fmt>    Output format: human, markdown, json, plain (default: auto)\n"
     << "  --color <mode>        Color mode: auto, always, never (or --no-color)\n\n"
     << "Commands:\n";

  for (const auto &cmd : commands_) {
    ss << std::format("  grm {:<18} {}\n", cmd.name, cmd.description);
  }

  ss << "\nRun 'grm <command> --help' for details on subcommands.\n";
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
    ss << std::format("  {:<66} {}\n", line, sub.description);
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

      ss << std::format("  {:<35} {}\n", flags, opt.description);
    }
  }

  return ss.str();
}

std::string CommandRegistry::render_all_help() const {
  std::stringstream ss;
  ss << "grm - Group & Telegram Manager CLI (C++23 / TDLib) - Exhaustive Master Help\n\n"
     << "Global Options:\n"
     << "  -h, --help            Show command help screen\n"
     << "  --help=all            Show exhaustive master help for all commands and options\n"
     << "  -V, --version         Display version and build environment info\n"
     << "  -v, --verbose         Enable verbose TDLib state output\n"
     << "  -d, --debug           Enable debug tracing\n"
     << "  -q, --quiet           Suppress non-error informational messages\n"
     << "  -c, --config <file>   Path to custom configuration file\n"
     << "  -T, --test-dc         Connect to Telegram Test Data Center environment\n"
     << "  -F, --format <fmt>    Output format: human, markdown, json, plain (default: auto)\n"
     << "  --color <mode>        Set ANSI color mode: auto, always, never (or --no-color)\n\n"
     << "================================================================================\n"
     << "EXHAUSTIVE COMMAND REFERENCE & SUBCOMMAND OPTIONS\n"
     << "================================================================================\n\n";

  for (const auto &cmd : commands_) {
    ss << std::format("Command: grm {}\n{}\n", cmd.name, cmd.description);
    ss << std::string(80, '-') << "\n\n";

    for (const auto &sub : cmd.subcommands) {
      std::string line = std::format("grm {} {} {}", cmd.name, sub.name, sub.synopsis);
      ss << std::format("  {:<66} {}\n", line, sub.description);

      if (!sub.options.empty()) {
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

          ss << std::format("      {:<33} {}\n", flags, opt_desc);
        }
      }
      ss << "\n";
    }
    ss << "\n";
  }

  return ss.str();
}

std::string CommandRegistry::render_completion(const std::string &shell) const {
  if (shell == "bash") {
    std::stringstream ss;
    ss << R"(#!/usr/bin/bash
# Bash completion script for grm (Group & Telegram Manager CLI)
# Generated dynamically by 'grm completion bash'

_grm_completions() {
  local cur prev words cword
  _init_completion || return

  local global_opts="-h --help -V --version -v --verbose -d --debug -q --quiet -c --config -T --test-dc -F --format --color --no-color"
  local commands="login chat msg topic file completion"

  if [[ ${cword} -eq 1 ]]; then
    if [[ "${cur}" == -* ]]; then
      COMPREPLY=($(compgen -W "${global_opts}" -- "${cur}"))
    else
      COMPREPLY=($(compgen -W "${commands}" -- "${cur}"))
    fi
    return 0
  fi

  local command="${words[1]}"

  case "${command}" in
    login)
      COMPREPLY=($(compgen -W "-p --phone -k --code -h --help" -- "${cur}"))
      ;;
    chat)
      if [[ ${cword} -eq 2 ]]; then
        if [[ "${cur}" == -* ]]; then
          COMPREPLY=($(compgen -W "-h --help" -- "${cur}"))
        else
          COMPREPLY=($(compgen -W "ls create info set-title set-desc pin unpin delete" -- "${cur}"))
        fi
      elif [[ "${words[2]}" == "ls" ]]; then
        COMPREPLY=($(compgen -W "-n --limit -S --since -f --filter -h --help" -- "${cur}"))
      elif [[ "${words[2]}" == "create" ]]; then
        if [[ ${cword} -eq 3 ]]; then
          COMPREPLY=($(compgen -W "group channel" -- "${cur}"))
        else
          COMPREPLY=($(compgen -W "--private --public -h --help" -- "${cur}"))
        fi
      elif [[ "${words[2]}" == "info" || "${words[2]}" == "delete" || "${words[2]}" == "pin" || "${words[2]}" == "unpin" || "${words[2]}" == "set-title" || "${words[2]}" == "set-desc" ]]; then
        COMPREPLY=($(compgen -W "-h --help" -- "${cur}"))
      fi
      ;;
    msg)
      if [[ ${cword} -eq 2 ]]; then
        if [[ "${cur}" == -* ]]; then
          COMPREPLY=($(compgen -W "-h --help" -- "${cur}"))
        else
          COMPREPLY=($(compgen -W "ls export search send info edit pin unpin delete" -- "${cur}"))
        fi
      elif [[ "${words[2]}" == "ls" ]]; then
        COMPREPLY=($(compgen -W "-n --limit -t --topic -S --since -f --filter --sender -r --reverse -h --help" -- "${cur}"))
      elif [[ "${words[2]}" == "export" ]]; then
        if [[ "${prev}" == "-f" || "${prev}" == "--format" ]]; then
          COMPREPLY=($(compgen -W "csv json" -- "${cur}"))
        elif [[ "${prev}" == "-o" || "${prev}" == "--output" ]]; then
          _filedir
        else
          COMPREPLY=($(compgen -W "-f --format -o --output -t --topic -n --limit -h --help" -- "${cur}"))
        fi
      elif [[ "${words[2]}" == "search" ]]; then
        COMPREPLY=($(compgen -W "-q --query -n --limit -t --topic -S --since -f --filter --sender -h --help" -- "${cur}"))
      elif [[ "${words[2]}" == "send" ]]; then
        if [[ "${prev}" == "-a" || "${prev}" == "--attach" ]]; then
          _filedir
        else
          COMPREPLY=($(compgen -W "-a --attach -m --media -C --caption -t --topic -h --help" -- "${cur}"))
        fi
      elif [[ "${words[2]}" == "edit" || "${words[2]}" == "info" || "${words[2]}" == "pin" ]]; then
        COMPREPLY=($(compgen -W "-t --topic -h --help" -- "${cur}"))
      elif [[ "${words[2]}" == "unpin" ]]; then
        COMPREPLY=($(compgen -W "-a --all -h --help" -- "${cur}"))
      elif [[ "${words[2]}" == "delete" ]]; then
        COMPREPLY=($(compgen -W "--for-everyone -e -h --help" -- "${cur}"))
      fi
      ;;
    topic)
      if [[ ${cword} -eq 2 ]]; then
        if [[ "${cur}" == -* ]]; then
          COMPREPLY=($(compgen -W "-h --help" -- "${cur}"))
        else
          COMPREPLY=($(compgen -W "ls create info edit close reopen pin unpin delete" -- "${cur}"))
        fi
      elif [[ "${words[2]}" == "ls" ]]; then
        COMPREPLY=($(compgen -W "-n --limit -S --since -f --filter -h --help" -- "${cur}"))
      elif [[ "${words[2]}" == "create" || "${words[2]}" == "edit" ]]; then
        COMPREPLY=($(compgen -W "-e --emoji --icon --icon-color -h --help" -- "${cur}"))
      elif [[ "${words[2]}" == "info" || "${words[2]}" == "close" || "${words[2]}" == "reopen" || "${words[2]}" == "pin" || "${words[2]}" == "unpin" || "${words[2]}" == "delete" ]]; then
        COMPREPLY=($(compgen -W "-h --help" -- "${cur}"))
      fi
      ;;
    file)
      if [[ ${cword} -eq 2 ]]; then
        if [[ "${cur}" == -* ]]; then
          COMPREPLY=($(compgen -W "-h --help" -- "${cur}"))
        else
          COMPREPLY=($(compgen -W "get" -- "${cur}"))
        fi
      elif [[ "${words[2]}" == "get" ]]; then
        if [[ "${prev}" == "--type" ]]; then
          COMPREPLY=($(compgen -W "photo video doc audio all" -- "${cur}"))
        elif [[ "${prev}" == "-o" || "${prev}" == "--output" ]]; then
          _filedir
        else
          COMPREPLY=($(compgen -W "-a --all -A -o --output -t --topic -n --limit --type -h --help" -- "${cur}"))
        fi
      fi
      ;;
    completion)
      COMPREPLY=($(compgen -W "bash zsh fish -h --help" -- "${cur}"))
      ;;
    *)
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
    ss << R"(#compdef grm
# Zsh completion script for grm (Group & Telegram Manager CLI)
# Generated dynamically by 'grm completion zsh'

_grm() {
  local -a commands
  commands=(
    'login:Authenticate Telegram account'
    'chat:Manage Telegram chats, groups, and channels'
    'msg:Inspect, send, edit, search, pin, and export messages'
    'topic:Manage supergroup forum topics'
    'file:Download media and file attachments'
    'completion:Generate shell autocompletion script'
  )

  _arguments -s \
    '(-h --help)'{-h,--help}'[Show help screen]' \
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
        file)
          _arguments '1:subcommand:(get)'
          ;;
        completion)
          _arguments '1:shell:(bash zsh fish)'
          ;;
      esac
      ;;
  esac
}

_grm "$@"
)";
    return ss.str();
  }

  if (shell == "fish") {
    std::stringstream ss;
    ss << R"(# Fish completion script for grm
complete -c grm -f -n '__fish_use_subcommand' -a 'login' -d 'Authenticate Telegram account'
complete -c grm -f -n '__fish_use_subcommand' -a 'chat' -d 'Manage Telegram chats'
complete -c grm -f -n '__fish_use_subcommand' -a 'msg' -d 'Manage messages'
complete -c grm -f -n '__fish_use_subcommand' -a 'topic' -d 'Manage forum topics'
complete -c grm -f -n '__fish_use_subcommand' -a 'file' -d 'Download attachments'
complete -c grm -f -n '__fish_use_subcommand' -a 'completion' -d 'Generate shell autocompletion script'
)";
    return ss.str();
  }

  return "";
}

} // namespace grm
