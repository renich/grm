#include "grm/app.hpp"
#include "grm/exporter.hpp"
#include "grm/json_utils.hpp"
#include "grm/logger.hpp"

#include <charconv>
#include <filesystem>
#include <format>
#include <iostream>
#include <regex>

namespace grm {

static std::expected<int64_t, std::string> parse_int64(std::string_view str) {
  int64_t val = 0;
  auto [ptr, ec] = std::from_chars(str.data(), str.data() + str.size(), val);
  if (ec != std::errc{} || ptr != str.data() + str.size()) {
    return std::unexpected("Invalid integer parameter: " + std::string(str));
  }
  return val;
}

static std::expected<int, std::string> parse_int32(std::string_view str) {
  int val = 0;
  auto [ptr, ec] = std::from_chars(str.data(), str.data() + str.size(), val);
  if (ec != std::errc{} || ptr != str.data() + str.size()) {
    return std::unexpected("Invalid integer parameter: " + std::string(str));
  }
  return val;
}

static std::string extract_message_text(const JsonValue &m) {
  auto content = m.get_object("content");
  if (!content) {
    return "";
  }

  auto type_str = content->get_type().value_or("");
  if (type_str == "messageText") {
    if (auto text_obj = content->get_object("text")) {
      return text_obj->get_string("text").value_or("");
    }
  } else if (type_str == "messagePhoto" || type_str == "messageDocument" ||
             type_str == "messageVideo" || type_str == "messageAudio") {
    if (auto caption_obj = content->get_object("caption")) {
      std::string cap = caption_obj->get_string("text").value_or("");
      return cap.empty() ? ("[" + type_str + "]") : cap;
    }
    return "[" + type_str + "]";
  } else if (!type_str.empty()) {
    return "[" + type_str + "]";
  }

  return "";
}

std::expected<int, std::string>
App::cmd_msg_ls(const std::vector<std::string> &args) {
  if (args.empty()) {
    return std::unexpected("Usage: grm msg ls <chat_id> [limit]");
  }

  auto chat_id_res = parse_int64(args[0]);
  if (!chat_id_res) {
    return std::unexpected("Invalid chat_id: " + args[0]);
  }
  const int64_t chat_id = *chat_id_res;

  int limit = 20;
  if (args.size() >= 2) {
    if (auto lim_res = parse_int32(args[1])) {
      limit = *lim_res;
    }
  }

  if (auto res = ensure_authenticated(); !res) {
    return std::unexpected(res.error());
  }

  ensure_chat_loaded(chat_id);

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
    std::string text = extract_message_text(m);
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

  auto chat_id_res = parse_int64(args[0]);
  if (!chat_id_res) {
    return std::unexpected("Invalid chat_id: " + args[0]);
  }
  const int64_t chat_id = *chat_id_res;

  const std::string &format_type = args[1];
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

  ensure_chat_loaded(chat_id);

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
    rec.sender = "0";
    if (auto sender_obj = m.get_object("sender_id")) {
      if (auto uid = sender_obj->get_int("user_id")) {
        rec.sender = std::to_string(*uid);
      } else if (auto cid = sender_obj->get_int("chat_id")) {
        rec.sender = std::to_string(*cid);
      }
    } else if (auto raw_sender = m.get_int("sender_id")) {
      rec.sender = std::to_string(*raw_sender);
    }
    rec.text = extract_message_text(m);
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

  grm::log::info(std::format("Successfully exported {} messages to {}",
                             records.size(), out_path.string()));

  return 0;
}

std::expected<int, std::string>
App::cmd_msg_search(const std::vector<std::string> &args) {
  if (args.size() < 2) {
    return std::unexpected("Usage: grm msg search <chat_id> \"<query>\"");
  }

  auto chat_id_res = parse_int64(args[0]);
  if (!chat_id_res) {
    return std::unexpected("Invalid chat_id: " + args[0]);
  }
  const int64_t chat_id = *chat_id_res;

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

  ensure_chat_loaded(chat_id);

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
    std::string text = extract_message_text(m);

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

  auto chat_id_res = parse_int64(args[0]);
  if (!chat_id_res) {
    return std::unexpected("Invalid chat_id: " + args[0]);
  }
  const int64_t chat_id = *chat_id_res;

  const std::string &message_text = args[1];
  int64_t message_thread_id = 0;

  for (size_t i = 2; i < args.size(); ++i) {
    if (args[i] == "--topic" && i + 1 < args.size()) {
      if (auto thread_res = parse_int64(args[i + 1])) {
        message_thread_id = *thread_res;
      }
      ++i;
    }
  }

  if (auto res = ensure_authenticated(); !res) {
    return std::unexpected(res.error());
  }

  ensure_chat_loaded(chat_id);

  const std::string escaped_message = escape_json_string(message_text);

  const std::string payload = std::format(
      R"({{
        "chat_id": {},
        "message_thread_id": {},
        "input_message_content": {{
          "@type": "inputMessageText",
          "text": {{
            "@type": "formattedText",
            "text": "{}"
          }}
        }}
      }})",
      chat_id, message_thread_id, escaped_message);

  auto res = client_->send_request("sendMessage", payload, 10.0);
  if (!res) {
    return std::unexpected("Failed to send message: " + res.error());
  }

  grm::log::info("Message sent successfully.");
  return 0;
}

} // namespace grm
