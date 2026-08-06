
grm - Group & Telegram Manager CLI (C++23 / TDLib)

Global Options:
  -h, --help            Show this help screen
  -V, --version         Display version and build info
  -v, --verbose         Enable verbose TDLib state output
  -d, --debug           Enable debug tracing
  -q, --quiet           Suppress informational messages
  -c, --config <file>   Custom configuration file path
  -T, --test-dc         Connect to Telegram Test Data Center environment
  -F, --format <fmt>    Output format: human, markdown, json, plain (default: auto)
  --color <mode>        Color mode: auto, always, never (or --no-color)

Commands:
  grm login [-p|--phone <num>] [-k|--code <code>]                  Authenticate Telegram account
  grm chat <subcommand>                                            Manage chats, groups, and channels
  grm topic <subcommand>                                           Manage supergroup forum topics
  grm msg <subcommand>                                             Inspect, send, edit, search, and export messages
  grm file <subcommand>                                            Download attachments and media

Run 'grm <command> --help' for details on subcommands.

