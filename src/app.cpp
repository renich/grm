#include "grm/app.hpp"
#include <chrono>
#include <format>
#include <iostream>
#include <regex>
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
  grm extract bday <chat_id>        Extract registered birthdays from chat history
  grm send <chat_id> "<message>"    Send a message to a chat or group

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

std::expected<int, std::string> App::cmd_login() {
  if (auto res = init_tdlib(); !res) {
    return std::unexpected(res.error());
  }

  std::cout << "Initializing login flow..." << std::endl;

  for (int attempt = 0; attempt < 100; ++attempt) {
    if (auth_state_ == "authorizationStateReady") {
      std::cout << "✓ Authenticated successfully as user!" << std::endl;
      return 0;
    }

    if (auth_state_ == "authorizationStateWaitPhoneNumber") {
      std::cout << "Enter your Telegram phone number (e.g. +521234567890): ";
      std::string phone;
      std::cin >> phone;

      const std::string payload =
          std::format(R"({{"phone_number": "{}"}})", phone);
      auto res =
          client_->send_request("setAuthenticationPhoneNumber", payload, 10.0);
      if (!res) {
        std::cerr << "Failed to set phone number: " << res.error() << std::endl;
      }
    } else if (auth_state_ == "authorizationStateWaitCode") {
      std::cout << "Enter the authentication code sent by Telegram: ";
      std::string code;
      std::cin >> code;

      const std::string payload = std::format(R"({{"code": "{}"}})", code);
      auto res =
          client_->send_request("checkAuthenticationCode", payload, 10.0);
      if (!res) {
        std::cerr << "Invalid code: " << res.error() << std::endl;
      }
    } else if (auth_state_ == "authorizationStateWaitPassword") {
      std::cout << "Enter your 2FA password: ";
      std::string password;
      std::cin >> password;

      const std::string payload =
          std::format(R"({{"password": "{}"}})", password);
      auto res =
          client_->send_request("checkAuthenticationPassword", payload, 10.0);
      if (!res) {
        std::cerr << "Invalid password: " << res.error() << std::endl;
      }
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(500));
  }

  return std::unexpected("Authentication loop timed out");
}

std::expected<int, std::string>
App::cmd_chat_ls([[maybe_unused]] const std::vector<std::string> &args) {
  if (auto res = ensure_authenticated(); !res) {
    return std::unexpected(res.error());
  }

  // Request chats list
  auto chats_res = client_->send_request("getChats", R"({"limit": 100})", 10.0);
  if (!chats_res) {
    return std::unexpected("Failed to get chats: " + chats_res.error());
  }

  auto chat_ids = chats_res->get_array("chat_ids");
  std::cout << std::format("{:<20} {:<15} {}\n", "CHAT ID", "TYPE", "TITLE");
  std::cout << std::string(60, '-') << "\n";

  for (const auto &id_val : chat_ids) {
    if (auto cid = id_val.get_int("")) {
      const std::string chat_req = std::format(R"({{"chat_id": {}}})", *cid);
      auto chat_info = client_->send_request("getChat", chat_req, 3.0);
      if (chat_info) {
        std::string title = chat_info->get_string("title").value_or("Private");
        std::string type_name = "Chat";
        if (auto type_obj = chat_info->get_object("type")) {
          type_name = type_obj->get_type().value_or("Chat");
        }
        std::cout << std::format("{:<20} {:<15} {}\n", *cid, type_name, title);
      }
    }
  }

  return 0;
}

std::expected<int, std::string>
App::cmd_msg_ls(const std::vector<std::string> &args) {
  if (args.empty()) {
    return std::unexpected("Usage: grm msg ls <chat_id> [limit]");
  }

  int64_t chat_id = 0;
  try {
    chat_id = std::stoll(args[0]);
  } catch (...) {
    return std::unexpected("Invalid chat_id: " + args[0]);
  }

  int limit = 20;
  if (args.size() >= 2) {
    try {
      limit = std::stoi(args[1]);
    } catch (...) {
    }
  }

  if (auto res = ensure_authenticated(); !res) {
    return std::unexpected(res.error());
  }

  const std::string payload = std::format(
      R"({{"chat_id": {}, "from_message_id": 0, "offset": 0, "limit": {}}})",
      chat_id, limit);

  auto res = client_->send_request("getChatHistory", payload, 10.0);
  if (!res) {
    return std::unexpected("Failed to get chat history: " + res.error());
  }

  auto msgs = res->get_array("messages");
  std::cout << std::format("Fetched {} messages for chat {}\n", msgs.size(),
                           chat_id);
  std::cout << std::string(60, '-') << "\n";

  for (const auto &m : msgs) {
    auto id = m.get_int("id").value_or(0);
    std::string text;
    if (auto content = m.get_object("content")) {
      if (auto text_obj = content->get_object("text")) {
        text = text_obj->get_string("text").value_or("");
      }
    }
    if (!text.empty()) {
      std::cout << std::format("[MsgID {}]: {}\n", id, text);
    }
  }

  return 0;
}

std::expected<int, std::string>
App::cmd_extract_bday(const std::vector<std::string> &args) {
  if (args.empty()) {
    return std::unexpected("Usage: grm extract bday <chat_id>");
  }

  int64_t chat_id = 0;
  try {
    chat_id = std::stoll(args[0]);
  } catch (...) {
    return std::unexpected("Invalid chat_id: " + args[0]);
  }

  if (auto res = ensure_authenticated(); !res) {
    return std::unexpected(res.error());
  }

  const std::string payload = std::format(
      R"({{"chat_id": {}, "from_message_id": 0, "offset": 0, "limit": 100}})",
      chat_id);

  auto res = client_->send_request("getChatHistory", payload, 10.0);
  if (!res) {
    return std::unexpected("Failed to get chat history: " + res.error());
  }

  auto msgs = res->get_array("messages");
  const std::regex bday_regex(
      R"((?i)(setbirthday|micumple|cumple|\b\d{1,2}[\/\.-]\d{1,2}([\/\.-]\d{2,4})?\b))");

  std::cout << std::format("Scanning {} messages for birthdays in chat {}\n",
                           msgs.size(), chat_id);
  std::cout << std::string(60, '=') << "\n";

  for (const auto &m : msgs) {
    auto id = m.get_int("id").value_or(0);
    std::string text;
    if (auto content = m.get_object("content")) {
      if (auto text_obj = content->get_object("text")) {
        text = text_obj->get_string("text").value_or("");
      }
    }

    if (!text.empty() && std::regex_search(text, bday_regex)) {
      std::cout << std::format("[MsgID {}]: {}\n", id, text);
    }
  }

  return 0;
}

std::expected<int, std::string>
App::cmd_send(const std::vector<std::string> &args) {
  if (args.size() < 2) {
    return std::unexpected("Usage: grm send <chat_id> \"<message>\"");
  }

  int64_t chat_id = 0;
  try {
    chat_id = std::stoll(args[0]);
  } catch (...) {
    return std::unexpected("Invalid chat_id: " + args[0]);
  }

  const std::string &message_text = args[1];

  if (auto res = ensure_authenticated(); !res) {
    return std::unexpected(res.error());
  }

  const std::string payload = std::format(
      R"({{
        "chat_id": {},
        "input_message_content": {{
          "@type": "inputMessageText",
          "text": {{
            "@type": "formattedText",
            "text": "{}"
          }}
        }}
      }})",
      chat_id, message_text);

  auto res = client_->send_request("sendMessage", payload, 10.0);
  if (!res) {
    return std::unexpected("Failed to send message: " + res.error());
  }

  std::cout << "✓ Message sent successfully!" << std::endl;
  return 0;
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
  if (cmd == "msg" && !sub_args.empty() && sub_args[0] == "ls") {
    return cmd_msg_ls(
        std::vector<std::string>(sub_args.begin() + 1, sub_args.end()));
  }
  if (cmd == "extract" && !sub_args.empty() && sub_args[0] == "bday") {
    return cmd_extract_bday(
        std::vector<std::string>(sub_args.begin() + 1, sub_args.end()));
  }
  if (cmd == "send") {
    return cmd_send(sub_args);
  }

  print_usage();
  return std::unexpected("Unknown command: " + cmd);
}

} // namespace grm
