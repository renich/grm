#include "grm/app.hpp"
#include <algorithm>
#include <chrono>
#include <format>
#include <iostream>
#include <thread>

namespace grm {

App::App(Config config, CliOptions options)
    : config_(std::move(config)), options_(std::move(options)) {}

void App::print_version() { std::cout << "grm 1.0.0 (C++23 / TDLib 1.8.66)\n"; }

void App::print_usage() {
  std::cout << R"(
grm - Group & Telegram Manager CLI (C++23 / TDLib)

Global Options:
  -h, --help            Show this help screen
  -V, --version         Display version and build info
  -v, --verbose         Enable verbose TDLib state output
  -d, --debug           Enable debug tracing
  -q, --quiet           Suppress informational messages
  -F, --format <fmt>    Output format: human, markdown, json, plain (default: auto)
  --color <mode>        Color mode: auto, always, never (or --no-color)

Commands:
  grm login [-p|--phone <num>] [-k|--code <code>]                  Interactive or non-interactive authentication
  grm chat ls                                                      List active chats (groups, channels, private)
  grm msg ls [-n|--limit <N>] <chat_id>                            List recent messages from a chat
  grm msg export [-f|--format csv|json] [-o|--output <file>] <chat_id> Export chat history to CSV or JSON file
  grm msg search [-q|--query "<query>"] [-n|--limit <N>] <chat_id> Search chat history using regex filter
  grm send <chat_id> "<message>"                                   Send a text message to a chat or group
  grm send file [-C|--caption "<text>"] [-t|--topic <id>] <chat_id> <file> Upload a local file or document to a chat
  grm topic ls <supergroup_id>                                     List active forum topics in a supergroup


)" << '\n';
}

bool App::is_help_requested(const std::vector<std::string> &args) {
  return std::ranges::any_of(args, [](const std::string &arg) {
    return arg == "-h" || arg == "--help";
  });
}

void App::print_login_help() {
  std::cout << R"(Usage: grm login [-p|--phone <number>] [-k|--code <code>]

Authenticate your Telegram account with TDLib.

Options:
  -p, --phone <number>   Pre-fill phone number in E.164 format (e.g. +12025550123)
  -k, --code <code>      Pre-fill authentication code for non-interactive logins
  -h, --help             Show this help screen
)" << '\n';
}

void App::print_chat_help() {
  std::cout << R"(Usage: grm chat ls

List all active Telegram chats, groups, channels, and private conversations.

Options:
  -h, --help             Show this help screen
)" << '\n';
}

void App::print_msg_help() {
  std::cout << R"(Usage: grm msg <subcommand> [options] [args]

Inspect and manage chat message history.

Subcommands:
  grm msg ls [-n|--limit <N>] <chat_id>                             List recent messages (default limit: 20)
  grm msg export [-f|--format csv|json] [-o|--output <file>] <chat_id> Export chat history to CSV or JSON file
  grm msg search [-q|--query "<query>"] [-n|--limit <N>] <chat_id> Search chat history using regex pattern filter

Options:
  -h, --help                                                        Show this help screen
)" << '\n';
}

void App::print_send_help() {
  std::cout << R"(Usage:
  grm send <chat_id> "<message>"
  grm send file [-C|--caption "<text>"] [-t|--topic <id>] <chat_id> <file_path>

Send text messages or upload local files/documents to a chat.

Options:
  -C, --caption "<text>"  Attach text caption to uploaded document
  -t, --topic <id>        Target a specific forum topic thread ID
  -h, --help              Show this help screen
)" << '\n';
}

void App::print_topic_help() {
  std::cout << R"(Usage: grm topic ls <supergroup_id>

List active forum topics in a Telegram supergroup.

Options:
  -h, --help              Show this help screen
)" << '\n';
}



std::string App::get_auth_state() const {
  std::scoped_lock lock(auth_mutex_);
  return auth_state_;
}

void App::update_auth_state(std::string state, bool closed) {
  {
    std::scoped_lock lock(auth_mutex_);
    auth_state_ = std::move(state);
    if (closed) {
      is_closed_ = true;
    }
  }
  auth_cv_.notify_all();
}

std::expected<void, std::string> App::init_tdlib() {
  if (client_) {
    return {};
  }

  client_ = std::make_unique<TdClient>(config_);

  client_->on_update([this](const JsonValue &update) {
    if (auto type = update.get_type()) {
      if (*type == "updateAuthorizationState") {
        if (auto state = update.get_object("authorization_state")) {
          if (auto sttype = state->get_type()) {
            grm::log::debug("[Auth State]: " + *sttype);
            if (*sttype == "authorizationStateWaitCode") {
              if (auto code_info = state->get_object("code_info")) {
                if (auto code_type_obj = code_info->get_object("type")) {
                  if (auto code_type = code_type_obj->get_type()) {
                    if (*code_type == "authenticationCodeTypeTelegramMessage") {
                      grm::log::auth("Code sent as an in-app message to your "
                                     "active Telegram client (Chat: Telegram "
                                     "Service Notifications).");
                    } else if (*code_type == "authenticationCodeTypeSms") {
                      grm::log::auth("Code sent via SMS to your phone number.");
                    } else if (*code_type == "authenticationCodeTypeCall") {
                      grm::log::auth("Code will be delivered via an automated "
                                     "phone call.");
                    } else {
                      grm::log::auth("Code delivery method: " + *code_type);
                    }
                  }
                }
              }
            }
            update_auth_state(*sttype, *sttype == "authorizationStateClosed");

            if (*sttype == "authorizationStateWaitTdlibParameters") {
              const auto db_path = options_.use_test_dc
                                       ? config_.config_dir / "tdlib_test_db"
                                       : config_.db_dir;
              if (options_.use_test_dc) {
                grm::log::info(
                    "Connecting to Telegram Test Data Center (Test DC)...");
              }

              const std::string params = std::format(
                  R"({{
                    "use_test_dc": {},
                    "database_directory": "{}",
                    "files_directory": "{}/files",
                    "database_encryption_key": "",
                    "use_file_database": true,
                    "use_chat_info_database": true,
                    "use_message_database": true,
                    "use_secret_chats": true,
                    "api_id": {},
                    "api_hash": "{}",
                    "system_language_code": "en",
                    "device_model": "grm CLI",
                    "system_version": "Linux x86_64",
                    "application_version": "10.9.1"
                  }})",
                  options_.use_test_dc ? "true" : "false", db_path.string(),
                  db_path.string(), config_.api_id, config_.api_hash);

              client_->send_async("setTdlibParameters", params);
            } else if (*sttype == "authorizationStateWaitEncryptionKey") {
              client_->send_async("checkDatabaseEncryptionKey",
                                  R"({"encryption_key": ""})");
            }
          }
        }
      }
    }
  });

  if (auto res = client_->start(); !res) {
    return std::unexpected(res.error());
  }

  // Trigger initial TDLib authorization state update
  client_->send_async("getAuthorizationState", "{}");

  return {};
}

std::expected<void, std::string> App::ensure_authenticated() {
  if (auto res = init_tdlib(); !res) {
    return std::unexpected(res.error());
  }

  // Block and wait up to 5 seconds for authorization state to resolve
  {
    std::unique_lock<std::mutex> lock(auth_mutex_);
    auth_cv_.wait_for(lock, std::chrono::seconds(5), [this] {
      return auth_state_ == "authorizationStateReady" ||
             auth_state_ == "authorizationStateWaitPhoneNumber" ||
             auth_state_ == "authorizationStateWaitCode" ||
             auth_state_ == "authorizationStateWaitPassword" || is_closed_;
    });
  }

  const std::string current_state = get_auth_state();

  if (current_state == "authorizationStateReady") {
    return {};
  }

  return std::unexpected(
      "Not authenticated. Please run 'grm login' to authenticate first.");
}

void App::ensure_chat_loaded(int64_t chat_id) {
  if (!client_) {
    return;
  }
  const std::string chat_req = std::format(R"({{"chat_id": {}}})", chat_id);
  auto chat_res = client_->send_request("getChat", chat_req, 3.0);
  if (!chat_res) {
    auto load_res =
        client_->send_request("loadChats", R"({"limit": 100})", 5.0);
    if (!load_res) {
      grm::log::debug("loadChats: " + load_res.error());
    }
    for (int i = 0; i < 10; ++i) {
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
      if (client_->send_request("getChat", chat_req, 2.0)) {
        break;
      }
    }
  }
}


std::expected<int, std::string> App::run(const std::vector<std::string> &args) {
  if (options_.version) {
    print_version();
    return 0;
  }

  if (args.empty()) {
    print_usage();
    return 0;
  }

  const std::string &cmd = args[0];

  if (cmd == "-h" || cmd == "--help") {
    print_usage();
    return 0;
  }

  std::vector<std::string> sub_args(args.begin() + 1, args.end());

  if (cmd == "login") {
    if (is_help_requested(sub_args)) {
      print_login_help();
      return 0;
    }
    return cmd_login();
  }
  if (cmd == "chat") {
    if (is_help_requested(sub_args)) {
      print_chat_help();
      return 0;
    }
    if (!sub_args.empty() && sub_args[0] == "ls") {
      return cmd_chat_ls(
          std::vector<std::string>(sub_args.begin() + 1, sub_args.end()));
    }
  }
  if (cmd == "msg") {
    if (is_help_requested(sub_args)) {
      print_msg_help();
      return 0;
    }
    if (!sub_args.empty()) {
      if (sub_args[0] == "ls") {
        return cmd_msg_ls(
            std::vector<std::string>(sub_args.begin() + 1, sub_args.end()));
      }
      if (sub_args[0] == "export") {
        return cmd_msg_export(
            std::vector<std::string>(sub_args.begin() + 1, sub_args.end()));
      }
      if (sub_args[0] == "search") {
        return cmd_msg_search(
            std::vector<std::string>(sub_args.begin() + 1, sub_args.end()));
      }
    }
  }


  if (cmd == "topic") {
    if (is_help_requested(sub_args)) {
      print_topic_help();
      return 0;
    }
    if (!sub_args.empty() && sub_args[0] == "ls") {
      return cmd_topic_ls(
          std::vector<std::string>(sub_args.begin() + 1, sub_args.end()));
    }
  }
  if (cmd == "send") {
    if (is_help_requested(sub_args)) {
      print_send_help();
      return 0;
    }
    if (!sub_args.empty() && sub_args[0] == "file") {
      return cmd_send_file(
          std::vector<std::string>(sub_args.begin() + 1, sub_args.end()));
    }
    return cmd_send(sub_args);
  }

  print_usage();
  return std::unexpected("Unknown command: " + cmd);
}

} // namespace grm
