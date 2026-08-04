#include "grm/app.hpp"
#include <format>
#include <iostream>

namespace grm {

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

} // namespace grm
