#include "grm/app.hpp"
#include <chrono>
#include <format>
#include <iostream>
#include <thread>

namespace grm {

App::App(Config config) : config_(std::move(config)) {}

void App::print_usage() {
  std::cout << R"(
grm - Group & Telegram Manager CLI (C++23 / TDLib)

Usage:
  grm login                         Interactive authentication login
  grm chat ls                       List active chats (groups, channels, private)
  grm msg ls <chat_id> [limit]      List recent messages from a chat
  grm msg export <chat_id> csv|json Export chat history to CSV or JSON file
  grm msg search <chat_id> "<query>" Search chat history using regex filter
  grm extract bday <chat_id>        Extract registered birthdays from chat history
  grm send <chat_id> "<message>"    Send a message to a chat or group
  grm send file <chat_id> <path>    Upload a local file or document to a chat
  grm topic ls <supergroup_id>      List active forum topics in a supergroup



)" << std::endl;
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
            auth_state_ = *sttype;
            if (*sttype == "authorizationStateClosed") {
              is_closed_ = true;
            }
          }
        }
      }
    }
  });

  if (auto res = client_->start(); !res) {
    return std::unexpected(res.error());
  }

  // Set parameters
  const std::string params = std::format(
      R"({{
        "api_id": {},
        "api_hash": "{}",
        "system_language_code": "en",
        "device_model": "Desktop",
        "system_version": "Fedora Linux",
        "application_version": "1.0",
        "database_directory": "{}",
        "use_message_database": true,
        "use_secret_chats": true
      }})",
      config_.api_id, config_.api_hash, config_.db_dir.string());

  auto res = client_->send_request("setTdlibParameters", params, 5.0);
  if (!res) {
    // Parameter setting error or already initialized
  }

  return {};
}

std::expected<void, std::string> App::ensure_authenticated() {
  if (auto res = init_tdlib(); !res) {
    return std::unexpected(res.error());
  }

  // Wait up to 3 seconds for authorization state to resolve
  for (int i = 0; i < 30; ++i) {
    if (!auth_state_.empty()) {
      break;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }

  if (auth_state_ == "authorizationStateReady") {
    return {};
  }

  if (auth_state_ == "authorizationStateWaitTdlibParameters") {
    const std::string params = std::format(
        R"({{
          "api_id": {},
          "api_hash": "{}",
          "system_language_code": "en",
          "device_model": "Desktop",
          "system_version": "Fedora Linux",
          "application_version": "1.0",
          "use_message_database": true,
          "use_secret_chats": true
        }})",
        config_.api_id, config_.api_hash, config_.db_dir.string());

    static_cast<void>(client_->send_request("setTdlibParameters", params, 5.0));
  }

  return {};
}

std::expected<int, std::string> App::run(const std::vector<std::string> &args) {
  if (args.empty()) {
    print_usage();
    return 0;
  }

  const std::string &cmd = args[0];
  std::vector<std::string> sub_args(args.begin() + 1, args.end());

  if (cmd == "login") {
    return cmd_login();
  }
  if (cmd == "chat" && !sub_args.empty() && sub_args[0] == "ls") {
    return cmd_chat_ls(
        std::vector<std::string>(sub_args.begin() + 1, sub_args.end()));
  }
  if (cmd == "msg" && !sub_args.empty()) {
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

  if (cmd == "extract" && !sub_args.empty() && sub_args[0] == "bday") {
    return cmd_extract_bday(
        std::vector<std::string>(sub_args.begin() + 1, sub_args.end()));
  }
  if (cmd == "topic" && !sub_args.empty() && sub_args[0] == "ls") {
    return cmd_topic_ls(
        std::vector<std::string>(sub_args.begin() + 1, sub_args.end()));
  }
  if (cmd == "send") {
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
