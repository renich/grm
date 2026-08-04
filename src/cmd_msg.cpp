#include "grm/app.hpp"
#include "grm/exporter.hpp"
#include <filesystem>
#include <format>
#include <iostream>
#include <regex>

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
App::cmd_msg_export(const std::vector<std::string> &args) {
  if (args.size() < 2) {
    return std::unexpected(
        "Usage: grm msg export <chat_id> csv|json [filename]");
  }

  int64_t chat_id = 0;
  try {
    chat_id = std::stoll(args[0]);
  } catch (...) {
    return std::unexpected("Invalid chat_id: " + args[0]);
  }

  const std::string format_type = args[1];
  if (format_type != "csv" && format_type != "json") {
    return std::unexpected("Format must be 'csv' or 'json'");
  }

  std::filesystem::path out_path =
      (args.size() >= 3) ? std::filesystem::path(args[2])
                         : std::filesystem::path(std::format(
                               "chat_{}_export.{}", chat_id, format_type));

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
  std::vector<MessageRecord> records;
  records.reserve(msgs.size());

  for (const auto &m : msgs) {
    MessageRecord rec;
    rec.id = m.get_int("id").value_or(0);
    rec.chat_id = chat_id;
    rec.date = m.get_int("date").value_or(0);
    rec.sender = std::to_string(m.get_int("sender_id").value_or(0));
    if (auto content = m.get_object("content")) {
      if (auto text_obj = content->get_object("text")) {
        rec.text = text_obj->get_string("text").value_or("");
      }
    }
    records.push_back(rec);
  }

  std::expected<void, std::string> export_res;
  if (format_type == "json") {
    export_res = Exporter::to_json(records, out_path);
  } else {
    export_res = Exporter::to_csv(records, out_path);
  }

  if (!export_res) {
    return std::unexpected(export_res.error());
  }

  std::cout << std::format("✓ Successfully exported {} messages to {}\n",
                           records.size(), out_path.string());
  return 0;
}

std::expected<int, std::string>
App::cmd_msg_search(const std::vector<std::string> &args) {
  if (args.size() < 2) {
    return std::unexpected("Usage: grm msg search <chat_id> \"<query>\"");
  }

  int64_t chat_id = 0;
  try {
    chat_id = std::stoll(args[0]);
  } catch (...) {
    return std::unexpected("Invalid chat_id: " + args[0]);
  }

  const std::string &query = args[1];
  std::regex search_regex;
  try {
    search_regex = std::regex(query, std::regex::icase);
  } catch (const std::regex_error &e) {
    return std::unexpected("Invalid regex pattern: " + std::string(e.what()));
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
  std::cout << std::format("Searching {} messages in chat {} for '{}'\n",
                           msgs.size(), chat_id, query);
  std::cout << std::string(60, '=') << "\n";

  int match_count = 0;
  for (const auto &m : msgs) {
    auto id = m.get_int("id").value_or(0);
    std::string text;
    if (auto content = m.get_object("content")) {
      if (auto text_obj = content->get_object("text")) {
        text = text_obj->get_string("text").value_or("");
      }
    }

    if (!text.empty() && std::regex_search(text, search_regex)) {
      std::cout << std::format("[MsgID {}]: {}\n", id, text);
      match_count++;
    }
  }

  std::cout << std::format("Found {} matching messages.\n", match_count);
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
