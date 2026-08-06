#include "grm/app.hpp"
#include "grm/formatter.hpp"
#include "grm/json_utils.hpp"
#include <charconv>
#include <format>
#include <iostream>
#include <regex>

namespace grm {

static std::expected<int64_t, std::string> parse_int64(std::string_view str) {
  int64_t val = 0;
  auto [ptr, ec] = std::from_chars(str.data(), str.data() + str.size(), val);
  if (ec != std::errc{} || ptr != str.data() + str.size()) {
    return std::unexpected("Invalid integer: " + std::string(str));
  }
  return val;
}

static std::expected<int32_t, std::string> parse_int32(std::string_view str) {
  int32_t val = 0;
  auto [ptr, ec] = std::from_chars(str.data(), str.data() + str.size(), val);
  if (ec != std::errc{} || ptr != str.data() + str.size()) {
    return std::unexpected("Invalid integer: " + std::string(str));
  }
  return val;
}

std::expected<int, std::string>
App::cmd_chat_ls(const std::vector<std::string> &args) {
  int limit = 100;
  int64_t since_timestamp = 0;
  std::vector<std::string> filter_patterns;

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
    }
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

  if (auto res = ensure_authenticated(); !res) {
    return std::unexpected(res.error());
  }

  auto load_res = client_->send_request("loadChats", R"({"limit": 100})", 5.0);
  if (!load_res) {
    grm::log::debug("loadChats: " + load_res.error());
  }

  auto chats_res = client_->send_request("getChats", R"({"limit": 100})", 10.0);
  if (!chats_res) {
    return std::unexpected("Failed to get chats: " + chats_res.error());
  }

  auto chat_ids = chats_res->get_array("chat_ids");
  std::vector<fmt::ChatItem> items;
  items.reserve(chat_ids.size());

  for (const auto &id_val : chat_ids) {
    if (auto cid = id_val.as_int64()) {
      const std::string chat_req = std::format(R"({{"chat_id": {}}})", *cid);
      auto chat_info = client_->send_request("getChat", chat_req, 3.0);
      if (chat_info) {
        std::string title = chat_info->get_string("title").value_or("Private");
        std::string type_name = "Chat";
        if (auto type_obj = chat_info->get_object("type")) {
          type_name = type_obj->get_type().value_or("Chat");
        }
        int32_t unread = static_cast<int32_t>(
            chat_info->get_int("unread_count").value_or(0));
        int64_t last_date = 0;
        if (auto last_msg = chat_info->get_object("last_message")) {
          last_date = last_msg->get_int("date").value_or(0);
        }

        if (since_timestamp > 0 && last_date < since_timestamp) {
          continue;
        }

        if (has_filter) {
          bool matches = std::regex_search(title, filter_regex) ||
                         std::regex_search(type_name, filter_regex) ||
                         std::regex_search(std::to_string(*cid), filter_regex);
          if (!matches) {
            continue;
          }
        }

        items.push_back(fmt::ChatItem{.id = *cid,
                                      .type = type_name,
                                      .title = title,
                                      .unread_count = unread,
                                      .last_message_date = last_date});
        if (items.size() >= static_cast<size_t>(limit)) {
          break;
        }
      }
    }
  }

  fmt::Formatter::render(items, "chat.ls", options_.format,
                         options_.color_mode);
  return 0;
}

std::expected<int, std::string>
App::cmd_chat_create(const std::vector<std::string> &args) {
  if (args.size() < 2) {
    return std::unexpected("Usage: grm chat create <group|channel> "
                           "[--private|--public] \"<title>\"");
  }

  const std::string &kind = args[0];
  const std::string &title = args[1];

  if (auto res = ensure_authenticated(); !res) {
    return std::unexpected(res.error());
  }

  std::string method = (kind == "channel") ? "createNewSupergroupChat"
                                           : "createNewBasicGroupChat";
  std::string payload;

  if (kind == "channel") {
    payload = std::format(
        R"({{
          "title": "{}",
          "is_forum": false,
          "is_channel": true,
          "description": ""
        }})",
        escape_json_string(title));
  } else {
    payload = std::format(
        R"({{
          "title": "{}"
        }})",
        escape_json_string(title));
  }

  auto res = client_->send_request(method, payload, 10.0);
  if (!res) {
    return std::unexpected("Failed to create " + kind + ": " + res.error());
  }

  grm::log::info(kind + " created successfully: " + title);
  return 0;
}

std::expected<int, std::string>
App::cmd_chat_info(const std::vector<std::string> &args) {
  if (args.empty()) {
    return std::unexpected("Usage: grm chat info <chat_id>");
  }

  auto cid_res = parse_int64(args[0]);
  if (!cid_res)
    return std::unexpected(cid_res.error());
  const int64_t chat_id = *cid_res;

  if (auto res = ensure_authenticated(); !res)
    return std::unexpected(res.error());
  ensure_chat_loaded(chat_id);

  const std::string req = std::format(R"({{"chat_id": {}}})", chat_id);
  auto chat_res = client_->send_request("getChat", req, 5.0);
  if (!chat_res) {
    return std::unexpected("Failed to get chat info: " + chat_res.error());
  }

  std::cout << chat_res->to_string() << '\n';
  return 0;
}

std::expected<int, std::string>
App::cmd_chat_set_title(const std::vector<std::string> &args) {
  if (args.size() < 2) {
    return std::unexpected(
        "Usage: grm chat set-title <chat_id> \"<new_title>\"");
  }

  auto cid_res = parse_int64(args[0]);
  if (!cid_res)
    return std::unexpected(cid_res.error());
  const int64_t chat_id = *cid_res;
  const std::string &title = args[1];

  if (auto res = ensure_authenticated(); !res)
    return std::unexpected(res.error());
  ensure_chat_loaded(chat_id);

  const std::string payload = std::format(
      R"({{
        "chat_id": {},
        "title": "{}"
      }})",
      chat_id, escape_json_string(title));

  auto res = client_->send_request("setChatTitle", payload, 10.0);
  if (!res) {
    return std::unexpected("Failed to set chat title: " + res.error());
  }

  grm::log::info("Chat title updated successfully.");
  return 0;
}

std::expected<int, std::string>
App::cmd_chat_set_desc(const std::vector<std::string> &args) {
  if (args.size() < 2) {
    return std::unexpected(
        "Usage: grm chat set-desc <chat_id> \"<description>\"");
  }

  auto cid_res = parse_int64(args[0]);
  if (!cid_res)
    return std::unexpected(cid_res.error());
  const int64_t chat_id = *cid_res;
  const std::string &desc = args[1];

  if (auto res = ensure_authenticated(); !res)
    return std::unexpected(res.error());
  ensure_chat_loaded(chat_id);

  const std::string payload = std::format(
      R"({{
        "chat_id": {},
        "description": "{}"
      }})",
      chat_id, escape_json_string(desc));

  auto res = client_->send_request("setChatDescription", payload, 10.0);
  if (!res) {
    return std::unexpected("Failed to set chat description: " + res.error());
  }

  grm::log::info("Chat description updated successfully.");
  return 0;
}

std::expected<int, std::string>
App::cmd_chat_pin(const std::vector<std::string> &args) {
  if (args.size() < 2) {
    return std::unexpected("Usage: grm chat pin <chat_id> <message_id>");
  }

  auto cid_res = parse_int64(args[0]);
  auto mid_res = parse_int64(args[1]);
  if (!cid_res || !mid_res)
    return std::unexpected("Invalid chat_id or message_id");

  if (auto res = ensure_authenticated(); !res)
    return std::unexpected(res.error());

  const std::string payload = std::format(
      R"({{
        "chat_id": {},
        "message_id": {},
        "disable_notification": false,
        "only_for_self": false
      }})",
      *cid_res, *mid_res);

  auto res = client_->send_request("pinChatMessage", payload, 10.0);
  if (!res) {
    return std::unexpected("Failed to pin message: " + res.error());
  }

  grm::log::info("Message pinned successfully.");
  return 0;
}

std::expected<int, std::string>
App::cmd_chat_unpin(const std::vector<std::string> &args) {
  if (args.empty()) {
    return std::unexpected("Usage: grm chat unpin <chat_id> [<message_id>]");
  }

  auto cid_res = parse_int64(args[0]);
  if (!cid_res)
    return std::unexpected(cid_res.error());
  int64_t message_id = (args.size() > 1) ? parse_int64(args[1]).value_or(0) : 0;

  if (auto res = ensure_authenticated(); !res)
    return std::unexpected(res.error());

  const std::string payload = std::format(
      R"({{
        "chat_id": {},
        "message_id": {}
      }})",
      *cid_res, message_id);

  auto res = client_->send_request("unpinChatMessage", payload, 10.0);
  if (!res) {
    return std::unexpected("Failed to unpin message: " + res.error());
  }

  grm::log::info("Message unpinned successfully.");
  return 0;
}

std::expected<int, std::string>
App::cmd_chat_delete(const std::vector<std::string> &args) {
  if (args.empty()) {
    return std::unexpected("Usage: grm chat delete <chat_id>");
  }

  auto cid_res = parse_int64(args[0]);
  if (!cid_res)
    return std::unexpected(cid_res.error());

  if (auto res = ensure_authenticated(); !res)
    return std::unexpected(res.error());

  const std::string payload = std::format(
      R"({{
        "chat_id": {},
        "remove_from_chat_list": true,
        "revoke": false
      }})",
      *cid_res);

  auto res = client_->send_request("deleteChatHistory", payload, 10.0);
  if (!res) {
    return std::unexpected("Failed to delete chat history: " + res.error());
  }

  grm::log::info("Chat deleted successfully.");
  return 0;
}

} // namespace grm
