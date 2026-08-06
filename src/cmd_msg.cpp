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
  int64_t topic_id = 0;

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
    } else if ((arg == "-t" || arg == "--topic") && i + 1 < args.size()) {
      if (auto tid = parse_int64(args[++i])) {
        topic_id = *tid;
      }
    } else if (arg.starts_with("--topic=")) {
      if (auto tid = parse_int64(arg.substr(8))) {
        topic_id = *tid;
      }
    } else if (!chat_id_set && parse_int64(arg).has_value()) {
      chat_id = *parse_int64(arg);
      chat_id_set = true;
    }
  }

  if (!chat_id_set) {
    return std::unexpected(
        "Usage: grm msg ls [-t|--topic <id>] [-n|--limit <N>] <chat_id>");
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
    auto fetch_limit =
        static_cast<int>(std::min<size_t>(100, target_limit - items.size()));

    const std::string method_name =
        (topic_id > 0) ? "getForumTopicHistory" : "getChatHistory";
    std::string payload;
    if (topic_id > 0) {
      payload = std::format(
          R"({{
            "chat_id": {},
            "forum_topic_id": {},
            "message_thread_id": {},
            "from_message_id": {},
            "offset": 0,
            "limit": {}
          }})",
          chat_id, topic_id, topic_id, from_msg_id, fetch_limit);
    } else {

      payload = std::format(
          R"({{
            "chat_id": {},
            "from_message_id": {},
            "offset": 0,
            "limit": {},
            "only_local": false
          }})",
          chat_id, from_msg_id, fetch_limit);
    }

    auto res = client_->send_request(method_name, payload, 10.0);

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
        int64_t msg_date = m.get_int("date").value_or(0);
        bool has_attach = false;
        std::string attach_type;
        if (auto content = m.get_object("content")) {
          std::string type_str = content->get_type().value_or("");
          if (type_str == "messageDocument") {
            has_attach = true;
            attach_type = "document";
          } else if (type_str == "messagePhoto") {
            has_attach = true;
            attach_type = "photo";
          } else if (type_str == "messageVideo") {
            has_attach = true;
            attach_type = "video";
          } else if (type_str == "messageAudio") {
            has_attach = true;
            attach_type = "audio";
          }
        }
        items.push_back(fmt::MessageItem{.id = id,
                                         .chat_id = chat_id,
                                         .topic_id = topic_id,
                                         .date = msg_date,
                                         .sender = "",
                                         .text = text,
                                         .has_attachment = has_attach,
                                         .attachment_type = attach_type});
        if (items.size() >= target_limit) {
          break;
        }
      }
    }
  }

  fmt::Formatter::render(items, "msg.ls", options_.format, options_.color_mode);
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
      int64_t msg_date = m.get_int("date").value_or(0);
      bool has_attach = false;
      std::string attach_type;
      if (auto content = m.get_object("content")) {
        std::string type_str = content->get_type().value_or("");
        if (type_str == "messageDocument") {
          has_attach = true;
          attach_type = "document";
        } else if (type_str == "messagePhoto") {
          has_attach = true;
          attach_type = "photo";
        } else if (type_str == "messageVideo") {
          has_attach = true;
          attach_type = "video";
        } else if (type_str == "messageAudio") {
          has_attach = true;
          attach_type = "audio";
        }
      }
      items.push_back(fmt::MessageItem{.id = id,
                                       .chat_id = chat_id,
                                       .topic_id = 0,
                                       .date = msg_date,
                                       .sender = "",
                                       .text = text,
                                       .has_attachment = has_attach,
                                       .attachment_type = attach_type});
      match_count++;
    }
  }

  fmt::Formatter::render(items, "msg.search", options_.format,
                         options_.color_mode);
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
  [[maybe_unused]] bool is_media = false;
  int64_t message_thread_id = 0;

  for (size_t i = 0; i < args.size(); ++i) {
    std::string_view arg(args[i]);
    if ((arg == "-a" || arg == "--attach" || arg == "-A" ||
         arg == "--attachment") &&
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
    auto text_obj = parse_formatted_text(message_text, "markdown");
    std::string text_json =
        text_obj ? text_obj->to_string()
                 : std::format(R"({{"@type":"formattedText","text":"{}"}})",
                               escape_json_string(message_text));
    std::string thread_part =
        (message_thread_id > 0)
            ? std::format(R"("message_thread_id": {},)", message_thread_id)
            : "";
    const std::string payload = std::format(
        R"({{
          "chat_id": {},
          {}
          "input_message_content": {{
            "@type": "inputMessageText",
            "text": {}
          }}
        }})",
        chat_id, thread_part, text_json);

    auto res = client_->send_request("sendMessage", payload, 10.0);
    if (!res) {
      return std::unexpected("Failed to send message: " + res.error());
    }
    int64_t sent_id = res->get_int("id").value_or(0);
    grm::log::info(std::format("Message sent successfully (ID: {}).", sent_id));
    return 0;
  }

  for (size_t idx = 0; idx < attachments.size(); ++idx) {
    const auto &file_path = attachments[idx];
    const std::string file_caption = (idx == 0) ? caption : "";

    std::error_code ec;
    if (!std::filesystem::exists(file_path, ec) ||
        !std::filesystem::is_regular_file(file_path, ec)) {
      return std::unexpected("Attachment file not found: " +
                             file_path.string());
    }

    const std::string abs_path =
        std::filesystem::absolute(file_path, ec).string();
    grm::log::info(
        std::format("Uploading {} to chat {}...", file_path.string(), chat_id));

    std::string caption_part;
    if (!file_caption.empty()) {
      auto caption_obj = parse_formatted_text(file_caption, "markdown");
      std::string caption_json =
          caption_obj
              ? caption_obj->to_string()
              : std::format(
                    R"({{"@type":"formattedText","text":"{}","entities":[]}})",
                    escape_json_string(file_caption));
      caption_part = std::format(R"(, "caption": {})", caption_json);
    }

    std::string thread_part;
    if (message_thread_id > 0) {
      thread_part =
          std::format(R"(, "message_thread_id": {})", message_thread_id);
    }

    std::string msg_payload;
    if (is_media) {
      msg_payload = std::format(
          R"({{
            "chat_id": {}{},
            "input_message_content": {{
              "@type": "inputMessagePhoto",
              "photo": {{
                "@type": "inputPhoto",
                "photo": {{
                  "@type": "inputFileLocal",
                  "path": "{}"
                }}
              }}{}
            }}
          }})",
          chat_id, thread_part, escape_json_string(abs_path), caption_part);
    } else {
      msg_payload = std::format(
          R"({{
            "chat_id": {}{},
            "input_message_content": {{
              "@type": "inputMessageDocument",
              "document": {{
                "@type": "inputDocument",
                "document": {{
                  "@type": "inputFileLocal",
                  "path": "{}"
                }}
              }}{}
            }}
          }})",
          chat_id, thread_part, escape_json_string(abs_path), caption_part);
    }

    auto res = client_->send_request("sendMessage", msg_payload, 30.0);
    if (!res) {
      return std::unexpected("Failed to send file " + file_path.string() +
                             ": " + res.error());
    }
  }

  grm::log::info("Attachment(s) sent successfully.");
  return 0;
}

std::expected<int, std::string>
App::cmd_msg_info(const std::vector<std::string> &args) {
  if (args.size() < 2) {
    return std::unexpected("Usage: grm msg info <chat_id> <message_id>");
  }

  auto cid_res = parse_int64(args[0]);
  auto mid_res = parse_int64(args[1]);
  if (!cid_res || !mid_res)
    return std::unexpected("Invalid chat_id or message_id");

  if (auto res = ensure_authenticated(); !res)
    return std::unexpected(res.error());
  ensure_chat_loaded(*cid_res);

  const std::string payload = std::format(
      R"({{
        "chat_id": {},
        "message_id": {}
      }})",
      *cid_res, *mid_res);

  auto res = client_->send_request("getMessage", payload, 5.0);
  if (!res) {
    return std::unexpected("Failed to get message info: " + res.error());
  }

  std::cout << res->to_string() << '\n';
  return 0;
}

std::expected<int, std::string>
App::cmd_msg_edit(const std::vector<std::string> &args) {
  if (args.size() < 3) {
    return std::unexpected("Usage: grm msg edit [-t|--topic <id>] <chat_id> "
                           "<message_id> \"<new_text>\"");
  }

  int64_t chat_id = 0;
  int64_t message_id = 0;
  std::string new_text;
  bool chat_set = false;
  bool msg_set = false;

  for (const auto &raw_arg : args) {
    std::string_view arg(raw_arg);
    if (!chat_set && parse_int64(arg).has_value()) {
      chat_id = *parse_int64(arg);
      chat_set = true;
    } else if (chat_set && !msg_set && parse_int64(arg).has_value()) {
      message_id = *parse_int64(arg);
      msg_set = true;
    } else if (chat_set && msg_set && new_text.empty() &&
               !arg.starts_with("-")) {
      new_text = arg;
    }
  }

  if (!chat_set || !msg_set || new_text.empty()) {
    return std::unexpected(
        "Usage: grm msg edit <chat_id> <message_id> \"<new_text>\"");
  }

  if (auto res = ensure_authenticated(); !res)
    return std::unexpected(res.error());

  const std::string payload = std::format(
      R"({{
        "chat_id": {},
        "message_id": {},
        "input_message_content": {{
          "@type": "inputMessageText",
          "text": {{
            "@type": "formattedText",
            "text": "{}"
          }}
        }}
      }})",
      chat_id, message_id, escape_json_string(new_text));

  auto res = client_->send_request("editMessageText", payload, 10.0);
  if (!res) {
    return std::unexpected("Failed to edit message: " + res.error());
  }

  grm::log::info("Message edited successfully.");
  return 0;
}

std::expected<int, std::string>
App::cmd_msg_delete(const std::vector<std::string> &args) {
  if (args.empty()) {
    return std::unexpected(
        "Usage: grm msg delete [--for-everyone] <chat_id> <message_ids...>");
  }

  bool revoke = false;
  int64_t chat_id = 0;
  bool chat_set = false;
  std::vector<int64_t> message_ids;

  for (const auto &raw_arg : args) {
    std::string_view arg(raw_arg);
    if (arg == "--for-everyone" || arg == "-e") {
      revoke = true;
    } else if (!chat_set && parse_int64(arg).has_value()) {
      chat_id = *parse_int64(arg);
      chat_set = true;
    } else if (chat_set && parse_int64(arg).has_value()) {
      message_ids.push_back(*parse_int64(arg));
    }
  }

  if (!chat_set || message_ids.empty()) {
    return std::unexpected(
        "Usage: grm msg delete [--for-everyone] <chat_id> <message_ids...>");
  }

  if (auto res = ensure_authenticated(); !res)
    return std::unexpected(res.error());

  std::string ids_json = "[";
  for (size_t idx = 0; idx < message_ids.size(); ++idx) {
    ids_json += std::to_string(message_ids[idx]);
    if (idx + 1 < message_ids.size())
      ids_json += ", ";
  }
  ids_json += "]";

  const std::string payload = std::format(
      R"({{
        "chat_id": {},
        "message_ids": {},
        "revoke": {}
      }})",
      chat_id, ids_json, revoke ? "true" : "false");

  auto res = client_->send_request("deleteMessages", payload, 10.0);
  if (!res) {
    return std::unexpected("Failed to delete messages: " + res.error());
  }

  grm::log::info("Message(s) deleted successfully.");
  return 0;
}

} // namespace grm
