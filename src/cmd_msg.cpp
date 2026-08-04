#include "grm/app.hpp"
#include "grm/exporter.hpp"
#include "grm/formatter.hpp"
#include "grm/json_utils.hpp"
#include "grm/logger.hpp"
#include "grm/uploader.hpp"



#include <charconv>
#include <chrono>
#include <filesystem>
#include <format>
#include <iostream>
#include <regex>
#include <thread>


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
  int limit = 20;
  int64_t chat_id = 0;
  bool chat_id_set = false;


  for (size_t i = 0; i < args.size(); ++i) {
    std::string_view arg(args[i]);
    if ((arg == "-n" || arg == "--limit") && i + 1 < args.size()) {
      if (auto lim = parse_int32(args[++i])) {
        limit = *lim;
      }
    } else if (arg.starts_with("--limit=")) {
      if (auto lim = parse_int32(arg.substr(8))) {
        limit = *lim;
      }
    } else if (!chat_id_set && parse_int64(arg).has_value()) {
      chat_id = *parse_int64(arg);
      chat_id_set = true;
    }

  }

  if (!chat_id_set) {
    return std::unexpected("Usage: grm msg ls <chat_id> [-n|--limit <N>]");
  }


  if (auto res = ensure_authenticated(); !res) {
    return std::unexpected(res.error());
  }

  ensure_chat_loaded(chat_id);

  std::vector<fmt::MessageItem> items;
  int64_t from_msg_id = 0;
  int empty_retries = 0;
  const size_t target_limit = static_cast<size_t>(std::max(1, limit));

  while (items.size() < target_limit) {
    auto fetch_limit = static_cast<int>(
        std::min<size_t>(target_limit - items.size(), 100));
    const std::string payload = std::format(
        R"({{"chat_id": {}, "from_message_id": {}, "offset": 0, "limit": {}}})",
        chat_id, from_msg_id, fetch_limit);

    auto res = client_->send_request("getChatHistory", payload, 10.0);
    if (!res) {
      if (items.empty()) {
        return std::unexpected("Failed to get chat history: " + res.error());
      }
      break;
    }

    auto batch = res->get_array("messages");
    if (batch.empty()) {
      empty_retries++;
      if (empty_retries > 2) {
        break;
      }
      std::this_thread::sleep_for(std::chrono::milliseconds(150));
      continue;
    }

    empty_retries = 0;
    for (const auto &m : batch) {
      auto id = m.get_int("id").value_or(0);
      if (id != 0) {
        from_msg_id = id;
      }
      std::string text = extract_message_text(m);
      if (!text.empty()) {
        items.push_back(fmt::MessageItem{
            .id = id, .chat_id = chat_id, .date = 0, .sender = "", .text = text});
        if (items.size() >= target_limit) {
          break;
        }
      }
    }
  }


  fmt::Formatter::print_messages(items, options_.format, options_.color_mode);
  return 0;



}


std::expected<int, std::string>
App::cmd_msg_export(const std::vector<std::string> &args) {
  int64_t chat_id = 0;
  bool chat_id_set = false;

  std::string format_type = "json";
  std::filesystem::path out_path;
  int export_limit = 1000;

  for (size_t i = 0; i < args.size(); ++i) {
    std::string_view arg(args[i]);
    if ((arg == "-f" || arg == "--format") && i + 1 < args.size()) {
      format_type = args[++i];
    } else if (arg.starts_with("--format=")) {
      format_type = arg.substr(9);
    } else if ((arg == "-o" || arg == "--output") && i + 1 < args.size()) {
      out_path = args[++i];
    } else if (arg.starts_with("--output=")) {
      out_path = arg.substr(9);
    } else if ((arg == "-n" || arg == "--limit") && i + 1 < args.size()) {
      if (auto lim = parse_int32(args[++i])) {
        export_limit = *lim;
      }
    } else if (arg.starts_with("--limit=")) {
      if (auto lim = parse_int32(arg.substr(8))) {
        export_limit = *lim;
      }
    } else if (!chat_id_set && parse_int64(arg).has_value()) {
      chat_id = *parse_int64(arg);
      chat_id_set = true;
    } else if (chat_id_set && (arg == "csv" || arg == "json")) {
      format_type = arg;
    } else if (chat_id_set && out_path.empty() && !arg.starts_with("-")) {
      out_path = arg;
    }
  }


  if (!chat_id_set) {
    return std::unexpected(
        "Usage: grm msg export <chat_id> [-f|--format csv|json] [-o|--output "
        "<file>] [-n|--limit <N>]");
  }

  if (out_path.empty()) {
    out_path = std::format("chat_{}_export.{}", chat_id, format_type);
  }


  if (auto res = ensure_authenticated(); !res) {
    return std::unexpected(res.error());
  }

  ensure_chat_loaded(chat_id);

  std::vector<MessageRecord> records;
  int64_t from_msg_id = 0;
  int empty_retries = 0;
  const size_t target_export_limit =
      static_cast<size_t>(std::max(1, export_limit));

  while (records.size() < target_export_limit) {
    auto fetch_limit = static_cast<int>(
        std::min<size_t>(target_export_limit - records.size(), 100));
    const std::string payload = std::format(
        R"({{"chat_id": {}, "from_message_id": {}, "offset": 0, "limit": {}}})",
        chat_id, from_msg_id, fetch_limit);


    auto res = client_->send_request("getChatHistory", payload, 10.0);
    if (!res) {
      if (records.empty()) {
        return std::unexpected("Failed to get chat history: " + res.error());
      }
      break;
    }

    auto batch = res->get_array("messages");
    if (batch.empty()) {
      empty_retries++;
      if (empty_retries > 2) {
        break;
      }
      std::this_thread::sleep_for(std::chrono::milliseconds(150));
      continue;
    }

    empty_retries = 0;
    for (const auto &m : batch) {
      MessageRecord rec;
      rec.id = m.get_int("id").value_or(0);
      if (rec.id != 0) {
        from_msg_id = rec.id;
      }
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
  int64_t chat_id = 0;
  bool chat_id_set = false;

  std::string query;
  int search_limit = 100;

  for (size_t i = 0; i < args.size(); ++i) {
    std::string_view arg(args[i]);
    if ((arg == "-q" || arg == "--query") && i + 1 < args.size()) {
      query = args[++i];
    } else if (arg.starts_with("--query=")) {
      query = arg.substr(8);
    } else if ((arg == "-n" || arg == "--limit") && i + 1 < args.size()) {
      if (auto lim = parse_int32(args[++i])) {
        search_limit = *lim;
      }
    } else if (arg.starts_with("--limit=")) {
      if (auto lim = parse_int32(arg.substr(8))) {
        search_limit = *lim;
      }
    } else if (!chat_id_set && parse_int64(arg).has_value()) {
      chat_id = *parse_int64(arg);
      chat_id_set = true;
    } else if (chat_id_set && query.empty() && !arg.starts_with("-")) {
      query = arg;
    }

  }

  if (!chat_id_set || query.empty()) {
    return std::unexpected(
        "Usage: grm msg search <chat_id> [-q|--query \"<query>\"] [-n|--limit "
        "<N>]");
  }

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
      R"({{"chat_id": {}, "from_message_id": 0, "offset": 0, "limit": {}}})",
      chat_id, search_limit);


  auto res = client_->send_request("getChatHistory", payload, 10.0);
  if (!res) {
    return std::unexpected("Failed to get chat history: " + res.error());
  }

  auto msgs = res->get_array("messages");
  std::cout << std::format("Searching {} messages in chat {} for '{}'\n",
                           msgs.size(), chat_id, query);
  std::cout << std::string(60, '=') << "\n";

  int match_count = 0;
  std::vector<fmt::MessageItem> items;
  for (const auto &m : msgs) {
    auto id = m.get_int("id").value_or(0);
    std::string text = extract_message_text(m);

    if (!text.empty() && std::regex_search(text, search_regex)) {
      items.push_back(fmt::MessageItem{
          .id = id, .chat_id = chat_id, .date = 0, .sender = "", .text = text});
      match_count++;
    }
  }

  fmt::Formatter::print_messages(items, options_.format, options_.color_mode);
  grm::log::info(std::format("Found {} matching messages.", match_count));
  return 0;

}

std::expected<int, std::string>
App::cmd_msg_send(const std::vector<std::string> &args) {
  int64_t chat_id = 0;
  bool chat_id_set = false;
  std::string message_text;
  std::string caption;
  std::vector<std::filesystem::path> attachments;
  bool is_media = false;
  int64_t message_thread_id = 0;

  for (size_t i = 0; i < args.size(); ++i) {
    std::string_view arg(args[i]);
    if ((arg == "-a" || arg == "--attach" || arg == "-A" || arg == "--attachment") &&
        i + 1 < args.size()) {
      attachments.emplace_back(args[++i]);
    } else if (arg.starts_with("--attach=")) {
      attachments.emplace_back(arg.substr(9));
    } else if (arg.starts_with("--attachment=")) {
      attachments.emplace_back(arg.substr(13));
    } else if (arg == "-m" || arg == "--media") {
      is_media = true;
    } else if ((arg == "-C" || arg == "--caption") && i + 1 < args.size()) {
      caption = args[++i];
    } else if (arg.starts_with("--caption=")) {
      caption = arg.substr(10);
    } else if ((arg == "-t" || arg == "--topic") && i + 1 < args.size()) {
      if (auto tid = parse_int64(args[++i])) {
        message_thread_id = *tid;
      }
    } else if (arg.starts_with("--topic=")) {
      if (auto tid = parse_int64(arg.substr(8))) {
        message_thread_id = *tid;
      }
    } else if (!chat_id_set && parse_int64(arg).has_value()) {
      chat_id = *parse_int64(arg);
      chat_id_set = true;
    } else if (chat_id_set && message_text.empty() && !arg.starts_with("-")) {
      message_text = arg;
    }
  }

  if (!chat_id_set) {
    return std::unexpected(
        "Usage: grm msg send [-a|--attach <file>] [-m|--media] [-C|--caption "
        "\"<text>\"] [-t|--topic <id>] <chat_id> [\"<message>\"]");
  }

  if (caption.empty() && !message_text.empty()) {
    caption = message_text;
  }

  if (auto res = ensure_authenticated(); !res) {
    return std::unexpected(res.error());
  }

  ensure_chat_loaded(chat_id);

  if (attachments.empty()) {
    if (message_text.empty()) {
      return std::unexpected(
          "Cannot send message: no text payload or file attachments provided.");
    }
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

  for (size_t idx = 0; idx < attachments.size(); ++idx) {
    const auto &file_path = attachments[idx];
    const std::string file_caption = (idx == 0) ? caption : "";

    std::expected<std::string, std::string> payload_res;
    if (is_media) {
      payload_res = Uploader::build_send_media_payload(
          chat_id, file_path, file_caption, message_thread_id);
    } else {
      payload_res = Uploader::build_send_document_payload(
          chat_id, file_path, file_caption, message_thread_id);
    }

    if (!payload_res) {
      return std::unexpected(payload_res.error());
    }

    grm::log::info(std::format("Uploading {} to chat {}...", file_path.string(),
                               chat_id));
    auto res = client_->send_request("sendMessage", *payload_res, 30.0);
    if (!res) {
      return std::unexpected("Failed to send file " + file_path.string() +
                             ": " + res.error());
    }
  }

  grm::log::info("Attachment(s) sent successfully.");
  return 0;
}


} // namespace grm
