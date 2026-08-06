#include "grm/app.hpp"
#include "grm/exporter.hpp"
#include "grm/formatter.hpp"
#include "grm/json_utils.hpp"
#include "grm/list_options.hpp"
#include "grm/logger.hpp"
#include "grm/uploader.hpp"

#include <charconv>
#include <chrono>
#include <filesystem>
#include <format>
#include <fstream>
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

SenderInfo App::resolve_sender_info(const JsonValue &message_obj) {
  auto sender_obj = message_obj.get_object("sender_id");
  if (!sender_obj) {
    return SenderInfo{};
  }

  std::string type_str = sender_obj->get_type().value_or("");
  if (type_str == "messageSenderUser") {
    int64_t user_id = sender_obj->get_int("user_id").value_or(0);
    if (user_id == 0) {
      return SenderInfo{};
    }

    if (auto it = sender_cache_.find(user_id); it != sender_cache_.end()) {
      return it->second;
    }

    std::string payload = std::format(R"({{"user_id": {}}})", user_id);
    auto res = client_->send_request("getUser", payload, 5.0);
    if (!res) {
      SenderInfo fallback{.chosen_name = std::format("User {}", user_id),
                          .full_name = "",
                          .username = "",
                          .id = user_id};
      sender_cache_[user_id] = fallback;
      return fallback;
    }

    std::string first_name = res->get_string("first_name").value_or("");
    std::string last_name = res->get_string("last_name").value_or("");
    std::string username = res->get_string("username").value_or("");
    if (username.empty()) {
      if (auto usernames_obj = res->get_object("usernames")) {
        username = usernames_obj->get_string("editable_username").value_or("");
        if (username.empty()) {
          auto active = usernames_obj->get_array("active_usernames");
          if (!active.empty()) {
            username = active[0].as_string().value_or("");
          }
        }
      }
    }

    std::string full_name = first_name;
    if (!last_name.empty()) {
      if (!full_name.empty()) {
        full_name += " ";
      }
      full_name += last_name;
    }

    std::string chosen_name;
    if (options_.name_format == NameFormat::Username) {
      if (!username.empty()) {
        chosen_name = "@" + username;
      } else if (!full_name.empty()) {
        chosen_name = full_name;
      } else {
        chosen_name = std::format("User {}", user_id);
      }
    } else {
      if (!full_name.empty()) {
        chosen_name = full_name;
      } else if (!username.empty()) {
        chosen_name = "@" + username;
      } else {
        chosen_name = std::format("User {}", user_id);
      }
    }

    SenderInfo info{.chosen_name = chosen_name,
                    .full_name = full_name,
                    .username = username,
                    .id = user_id};
    sender_cache_[user_id] = info;
    return info;
  } else if (type_str == "messageSenderChat") {
    int64_t sender_chat_id = sender_obj->get_int("chat_id").value_or(0);
    if (sender_chat_id == 0) {
      return SenderInfo{};
    }

    if (auto it = sender_cache_.find(sender_chat_id);
        it != sender_cache_.end()) {
      return it->second;
    }

    std::string payload = std::format(R"({{"chat_id": {}}})", sender_chat_id);
    auto res = client_->send_request("getChat", payload, 5.0);
    if (!res) {
      SenderInfo fallback{.chosen_name = std::format("Chat {}", sender_chat_id),
                          .full_name = "",
                          .username = "",
                          .id = sender_chat_id};
      sender_cache_[sender_chat_id] = fallback;
      return fallback;
    }

    std::string title = res->get_string("title").value_or("");
    std::string name =
        title.empty() ? std::format("Chat {}", sender_chat_id) : title;
    SenderInfo info{.chosen_name = name,
                    .full_name = name,
                    .username = "",
                    .id = sender_chat_id};
    sender_cache_[sender_chat_id] = info;
    return info;
  }

  return SenderInfo{};
}

std::string App::resolve_sender_name(const JsonValue &message_obj) {
  return resolve_sender_info(message_obj).chosen_name;
}

std::expected<int64_t, std::string>
App::parse_since_timestamp(std::string_view raw_str) {
  if (raw_str.empty()) {
    return std::unexpected("Empty since duration/date string");
  }

  size_t start = raw_str.find_first_not_of(" \t\n\r");
  size_t end = raw_str.find_last_not_of(" \t\n\r");
  if (start == std::string_view::npos) {
    return std::unexpected("Empty since duration/date string");
  }
  std::string str(raw_str.substr(start, end - start + 1));
  for (char &c : str) {
    c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
  }

  if (str.ends_with(" ago")) {
    str = str.substr(0, str.size() - 4);
    size_t e = str.find_last_not_of(" \t");
    if (e != std::string::npos) {
      str = str.substr(0, e + 1);
    }
  }

  const int64_t now_sec =
      std::chrono::duration_cast<std::chrono::seconds>(
          std::chrono::system_clock::now().time_since_epoch())
          .count();

  if (str == "yesterday") {
    return now_sec - 86400;
  }
  if (str == "today") {
    std::time_t t_now = static_cast<std::time_t>(now_sec);
    std::tm tm_now = {};
    gmtime_r(&t_now, &tm_now);
    tm_now.tm_hour = 0;
    tm_now.tm_min = 0;
    tm_now.tm_sec = 0;
    return static_cast<int64_t>(timegm(&tm_now));
  }

  if (str.find_first_not_of("0123456789") == std::string::npos &&
      str.size() >= 9) {
    if (auto val = parse_int64(str)) {
      return *val;
    }
  }

  int64_t num = 0;
  size_t idx = 0;
  while (idx < str.size() &&
         std::isdigit(static_cast<unsigned char>(str[idx]))) {
    num = num * 10 + (str[idx] - '0');
    idx++;
  }

  if (idx > 0) {
    while (idx < str.size() && (str[idx] == ' ' || str[idx] == '\t')) {
      idx++;
    }

    if (idx < str.size()) {
      std::string_view unit_str = std::string_view(str).substr(idx);
      int64_t multiplier = 0;
      if (unit_str == "s" || unit_str == "sec" || unit_str == "second" ||
          unit_str == "seconds") {
        multiplier = 1;
      } else if (unit_str == "m" || unit_str == "min" || unit_str == "minute" ||
                 unit_str == "minutes") {
        multiplier = 60;
      } else if (unit_str == "h" || unit_str == "hr" || unit_str == "hour" ||
                 unit_str == "hours") {
        multiplier = 3600;
      } else if (unit_str == "d" || unit_str == "day" || unit_str == "days") {
        multiplier = 86400;
      } else if (unit_str == "w" || unit_str == "week" ||
                 unit_str == "weeks") {
        multiplier = 604800;
      } else if (unit_str == "mon" || unit_str == "month" ||
                 unit_str == "months") {
        multiplier = 2592000;
      } else if (unit_str == "y" || unit_str == "year" ||
                 unit_str == "years") {
        multiplier = 31536000;
      }

      if (multiplier > 0) {
        return now_sec - (num * multiplier);
      }
    }
  }

  std::tm tm_buf = {};
  if (sscanf(str.data(), "%4d-%2d-%2d", &tm_buf.tm_year, &tm_buf.tm_mon,
             &tm_buf.tm_mday) == 3) {
    tm_buf.tm_year -= 1900;
    tm_buf.tm_mon -= 1;
    if (str.size() >= 19) {
      sscanf(str.data() + 11, "%2d:%2d:%2d", &tm_buf.tm_hour, &tm_buf.tm_min,
             &tm_buf.tm_sec);
    }
    time_t t = timegm(&tm_buf);
    if (t != -1) {
      return static_cast<int64_t>(t);
    }
  }

  return std::unexpected(
      std::format("Invalid since format '{}'. Expected duration (e.g. '1 day "
                  "ago', '3 days ago', '2h', '30m'), ISO date (YYYY-MM-DD), or "
                  "timestamp.",
                  raw_str));
}

std::expected<int, std::string>
App::cmd_msg_ls(const std::vector<std::string> &args) {
  std::vector<std::string> positionals;
  auto opts_res = ListOptions::parse(args, positionals);
  if (!opts_res) {
    return std::unexpected(opts_res.error());
  }
  const auto &opts = *opts_res;

  if (positionals.empty()) {
    return std::unexpected(
        "Usage: grm msg ls [-t|--topic <id>] [-n|--limit <N>] [--since "
        "<duration|date>] [-f|--filter <pattern>]... [-r|--reverse] <chat_id>");
  }

  auto cid_res = parse_int64(positionals[0]);
  if (!cid_res) {
    return std::unexpected(cid_res.error());
  }
  const int64_t chat_id = *cid_res;
  const int64_t topic_id = opts.topic_id;

  if (auto res = ensure_authenticated(); !res) {
    return std::unexpected(res.error());
  }

  ensure_chat_loaded(chat_id);

  std::vector<fmt::MessageItem> items;
  int64_t from_msg_id = 0;
  int empty_retries = 0;
  const size_t target_limit = static_cast<size_t>(std::max(1, opts.limit));

  bool reached_since_cutoff = false;
  while (!reached_since_cutoff) {
    constexpr int fetch_limit = 100;

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

      int64_t msg_date = m.get_int("date").value_or(0);
      if (!opts.matches_since(msg_date)) {
        reached_since_cutoff = true;
        break;
      }

      SenderInfo info = resolve_sender_info(m);
      if (opts.has_filter) {
        bool matches_filter = opts.matches_filter_multi(
            {info.chosen_name, info.username, "@" + info.username, info.full_name,
             info.id != 0 ? std::to_string(info.id) : ""});
        if (!matches_filter) {
          continue;
        }
      }

      std::string sender_name = info.chosen_name;

      std::string text = extract_message_text(m);
      if (!text.empty()) {
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
                                         .sender = sender_name,
                                         .text = text,
                                         .has_attachment = has_attach,
                                         .attachment_type = attach_type});
        if (opts.since_timestamp == 0 && items.size() >= target_limit) {
          break;
        }
      }
    }
  }

  std::sort(items.begin(), items.end(),
            [](const fmt::MessageItem &a, const fmt::MessageItem &b) {
              if (a.date != b.date)
                return a.date < b.date;
              return a.id < b.id;
            });
  auto last = std::unique(
      items.begin(), items.end(),
      [](const fmt::MessageItem &a, const fmt::MessageItem &b) {
        return a.id == b.id;
      });
  items.erase(last, items.end());

  if (opts.since_timestamp > 0 && items.size() > target_limit) {
    items.resize(target_limit);
  }

  if (opts.reverse_order) {
    std::reverse(items.begin(), items.end());
  }

  fmt::Formatter::render(
      items, "msg.ls", options_.format, options_.color_mode, std::cout,
      (options_.verbosity >= log::VerbosityLevel::Verbose));
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
    return std::unexpected("Usage: grm msg export [-f json|markdown] [-o file] "
                           "[-n limit] <chat_id>");
  }

  if (auto res = ensure_authenticated(); !res) {
    return std::unexpected(res.error());
  }

  ensure_chat_loaded(chat_id);

  std::vector<fmt::MessageItem> items;
  int64_t from_msg_id = 0;
  int empty_retries = 0;

  while (items.size() < static_cast<size_t>(export_limit)) {
    const std::string payload = std::format(
        R"({{
          "chat_id": {},
          "from_message_id": {},
          "offset": 0,
          "limit": 100,
          "only_local": false
        }})",
        chat_id, from_msg_id);

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
        int64_t msg_date = m.get_int("date").value_or(0);
        items.push_back(fmt::MessageItem{.id = id,
                                         .chat_id = chat_id,
                                         .topic_id = 0,
                                         .date = msg_date,
                                         .sender = resolve_sender_name(m),
                                         .text = text,
                                         .has_attachment = false,
                                         .attachment_type = ""});
        if (items.size() >= static_cast<size_t>(export_limit)) {
          break;
        }
      }
    }
  }

  std::ofstream out_file;
  std::ostream *out_stream = &std::cout;
  if (!out_path.empty()) {
    out_file.open(out_path);
    if (!out_file.is_open()) {
      return std::unexpected("Failed to open output file: " +
                             out_path.string());
    }
    out_stream = &out_file;
  }

  fmt::OutputFormat format = fmt::OutputFormat::Json;
  if (format_type == "markdown") {
    format = fmt::OutputFormat::Markdown;
  }

  fmt::Formatter::render(items, "msg.export", format, options_.color_mode,
                         *out_stream,
                         (options_.verbosity >= log::VerbosityLevel::Verbose));
  grm::log::info(std::format("Exported {} messages.", items.size()));
  return 0;
}

std::expected<int, std::string>
App::cmd_msg_search(const std::vector<std::string> &args) {
  int64_t chat_id = 0;
  bool chat_id_set = false;

  std::string query;
  int search_limit = 100;

  int64_t since_timestamp = 0;
  std::vector<std::string> filter_patterns;

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
    } else if ((arg == "-S" || arg == "--since") && i + 1 < args.size()) {
      if (auto ts = parse_since_timestamp(args[++i])) {
        since_timestamp = *ts;
      } else {
        return std::unexpected(ts.error());
      }
    } else if (arg.starts_with("--since=")) {
      if (auto ts = parse_since_timestamp(arg.substr(8))) {
        since_timestamp = *ts;
      } else {
        return std::unexpected(ts.error());
      }
    } else if ((arg == "-f" || arg == "--filter" || arg == "--sender") &&
               i + 1 < args.size()) {
      filter_patterns.push_back(args[++i]);
    } else if (arg.starts_with("--filter=")) {
      filter_patterns.push_back(std::string(arg.substr(9)));
    } else if (arg.starts_with("--sender=")) {
      filter_patterns.push_back(std::string(arg.substr(9)));
    } else if (!chat_id_set && parse_int64(arg).has_value()) {
      chat_id = *parse_int64(arg);
      chat_id_set = true;
    } else if (query.empty() && !arg.starts_with("-")) {
      query = arg;
    }
  }

  if (!chat_id_set || query.empty()) {
    return std::unexpected(
        "Usage: grm msg search <chat_id> [-q|--query \"<query>\"] [-n|--limit "
        "<N>] [--since <duration|date>] [-f|--filter <pattern>]...");
  }

  std::regex filter_regex;
  bool has_filter = false;
  if (!filter_patterns.empty()) {
    std::string combined;
    for (size_t i = 0; i < filter_patterns.size(); ++i) {
      if (i > 0) {
        combined += "|";
      }
      combined += std::format("({})", filter_patterns[i]);
    }
    try {
      filter_regex = std::regex(combined, std::regex::icase);
      has_filter = true;
    } catch (const std::regex_error &e) {
      return std::unexpected("Invalid filter pattern: " +
                             std::string(e.what()));
    }
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
  int match_count = 0;
  std::vector<fmt::MessageItem> items;
  for (const auto &m : msgs) {
    auto id = m.get_int("id").value_or(0);
    int64_t msg_date = m.get_int("date").value_or(0);
    if (since_timestamp > 0 && msg_date < since_timestamp) {
      continue;
    }

    SenderInfo info = resolve_sender_info(m);
    if (has_filter) {
      bool matches_filter =
          std::regex_search(info.chosen_name, filter_regex) ||
          (!info.username.empty() &&
           std::regex_search(info.username, filter_regex)) ||
          (!info.username.empty() &&
           std::regex_search("@" + info.username, filter_regex)) ||
          (!info.full_name.empty() &&
           std::regex_search(info.full_name, filter_regex)) ||
          (info.id != 0 &&
           std::regex_search(std::to_string(info.id), filter_regex));
      if (!matches_filter) {
        continue;
      }
    }

    std::string sender_name = info.chosen_name;

    std::string text = extract_message_text(m);

    if (!text.empty() && std::regex_search(text, search_regex)) {
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
                                       .sender = sender_name,
                                       .text = text,
                                       .has_attachment = has_attach,
                                       .attachment_type = attach_type});
      match_count++;
    }
  }

  fmt::Formatter::render(items, "msg.search", options_.format,
                         options_.color_mode, std::cout,
                         (options_.verbosity >= log::VerbosityLevel::Verbose));
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
