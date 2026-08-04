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

std::string App::get_auth_state() const {
  std::lock_guard<std::mutex> lock(auth_mutex_);
  return auth_state_;
}

void App::update_auth_state(std::string state, bool closed) {
  {
    std::lock_guard<std::mutex> lock(auth_mutex_);
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
            update_auth_state(*sttype, *sttype == "authorizationStateClosed");
            if (*sttype == "authorizationStateWaitTdlibParameters") {
              const std::string params = std::format(
                  R"({{
                    "parameters": {{
                      "@type": "tdlibParameters",
                      "use_test_dc": false,
                      "database_directory": "{}",
                      "files_directory": "{}/files",
                      "use_file_database": true,
                      "use_chat_info_database": true,
                      "use_message_database": true,
                      "use_secret_chats": true,
                      "api_id": {},
                      "api_hash": "{}",
                      "system_language_code": "en",
                      "device_model": "Desktop",
                      "system_version": "Fedora Linux",
                      "application_version": "1.0",
                      "enable_storage_optimizer": true
                    }}
                  }})",
                  config_.db_dir.string(), config_.db_dir.string(),
                  config_.api_id, config_.api_hash);

              client_->send_async("setTdlibParameters", params);
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
