#include "grm/app.hpp"
#include "grm/formatter.hpp"
#include "grm/json_utils.hpp"
#include "grm/logger.hpp"
#include <charconv>
#include <format>
#include <iostream>

namespace grm {

static std::expected<int64_t, std::string> parse_int64(std::string_view str) {
  int64_t val = 0;
  auto [ptr, ec] = std::from_chars(str.data(), str.data() + str.size(), val);
  if (ec != std::errc{} || ptr != str.data() + str.size()) {
    return std::unexpected("Invalid integer: " + std::string(str));
  }
  return val;
}

std::expected<int, std::string>
App::cmd_topic_ls(const std::vector<std::string> &args) {
  if (args.empty()) {
    return std::unexpected("Usage: grm topic ls <supergroup_id>");
  }

  auto cid_res = parse_int64(args[0]);
  if (!cid_res) return std::unexpected(cid_res.error());
  const int64_t chat_id = *cid_res;

  if (auto res = ensure_authenticated(); !res) {
    return std::unexpected(res.error());
  }

  ensure_chat_loaded(chat_id);

  const std::string payload = std::format(
      R"({{
        "chat_id": {},
        "query": "",
        "offset_date": 0,
        "offset_message_id": 0,
        "offset_message_thread_id": 0,
        "limit": 100
      }})",
      chat_id);

  auto res = client_->send_request("getForumTopics", payload, 10.0);
  if (!res) {
    return std::unexpected("Failed to get forum topics: " + res.error());
  }

  auto topics = res->get_array("topics");
  std::vector<fmt::TopicItem> items;
  items.reserve(topics.size());

  for (const auto &t : topics) {
    log::debug("Topic JSON: " + t.to_string());
    int64_t thread_id = 0;
    std::string name;
    int64_t total_messages = t.get_int("total_message_count").value_or(0);

    if (auto info = t.get_object("info")) {
      if (auto tid = info->get_int("message_thread_id")) {
        thread_id = *tid;
      } else if (auto fid = info->get_int("forum_topic_id")) {
        thread_id = *fid;
      }
      name = info->get_string("name").value_or("");
    }

    if (thread_id == 0) {
      if (auto fid = t.get_int("forum_topic_id")) {
        thread_id = *fid;
      } else if (auto tid = t.get_int("message_thread_id")) {
        thread_id = *tid;
      }
    }

    if (name.empty()) {
      name = t.get_string("name").value_or("General");
    }

    items.push_back(fmt::TopicItem{.id = thread_id,
                                   .name = name,
                                   .message_count = total_messages,
                                   .custom_emoji_id = 0,
                                   .icon_color = 0});
  }

  fmt::Formatter::print_topics(items, options_.format, options_.color_mode);
  return 0;
}

std::expected<int, std::string>
App::cmd_topic_create(const std::vector<std::string> &args) {
  std::string icon_emoji_id = "0";
  std::vector<std::string> positional;
  for (size_t i = 0; i < args.size(); ++i) {
    if ((args[i] == "-e" || args[i] == "--emoji" || args[i] == "--icon") &&
        i + 1 < args.size()) {
      icon_emoji_id = args[++i];
    } else {
      positional.push_back(args[i]);
    }
  }

  if (positional.size() < 2) {
    return std::unexpected(
        "Usage: grm topic create [-e|--emoji <custom_emoji_id>] <supergroup_id> \"<topic_name>\"");
  }

  auto cid_res = parse_int64(positional[0]);
  if (!cid_res) return std::unexpected(cid_res.error());
  const std::string &name = positional[1];

  if (auto res = ensure_authenticated(); !res) return std::unexpected(res.error());
  ensure_chat_loaded(*cid_res);

  const std::string payload = std::format(
      R"({{
        "chat_id": {},
        "name": "{}",
        "icon_custom_emoji_id": "{}"
      }})",
      *cid_res, escape_json_string(name), icon_emoji_id);

  auto res = client_->send_request("createForumTopic", payload, 10.0);
  if (!res) {
    return std::unexpected("Failed to create topic: " + res.error());
  }

  grm::log::info("Forum topic created successfully: " + name);
  return 0;
}

std::expected<int, std::string>
App::cmd_topic_info(const std::vector<std::string> &args) {
  if (args.size() < 2) {
    return std::unexpected("Usage: grm topic info <supergroup_id> <topic_id>");
  }

  auto cid_res = parse_int64(args[0]);
  auto tid_res = parse_int64(args[1]);
  if (!cid_res || !tid_res) return std::unexpected("Invalid supergroup_id or topic_id");

  if (auto res = ensure_authenticated(); !res) return std::unexpected(res.error());
  ensure_chat_loaded(*cid_res);

  const std::string payload = std::format(
      R"({{
        "chat_id": {},
        "message_thread_id": {},
        "forum_topic_id": {}
      }})",
      *cid_res, *tid_res, *tid_res);

  auto res = client_->send_request("getForumTopic", payload, 5.0);
  if (!res) {
    return std::unexpected("Failed to get topic info: " + res.error());
  }

  std::cout << res->to_string() << '\n';
  return 0;
}

std::expected<int, std::string>
App::cmd_topic_edit(const std::vector<std::string> &args) {
  std::string icon_emoji_id = "0";
  bool has_icon = false;
  std::vector<std::string> positional;
  for (size_t i = 0; i < args.size(); ++i) {
    if ((args[i] == "-e" || args[i] == "--emoji" || args[i] == "--icon") &&
        i + 1 < args.size()) {
      icon_emoji_id = args[++i];
      has_icon = true;
    } else {
      positional.push_back(args[i]);
    }
  }

  if (positional.size() < 2) {
    return std::unexpected(
        "Usage: grm topic edit [-e|--emoji <custom_emoji_id>] <supergroup_id> <topic_id> [\"<new_name>\"]");
  }

  auto cid_res = parse_int64(positional[0]);
  auto tid_res = parse_int64(positional[1]);
  if (!cid_res || !tid_res) return std::unexpected("Invalid supergroup_id or topic_id");
  const std::string new_name = positional.size() >= 3 ? positional[2] : "";

  if (auto res = ensure_authenticated(); !res) return std::unexpected(res.error());

  const std::string payload = std::format(
      R"({{
        "chat_id": {},
        "message_thread_id": {},
        "forum_topic_id": {},
        "name": "{}",
        "edit_icon_custom_emoji": {},
        "icon_custom_emoji_id": "{}"
      }})",
      *cid_res, *tid_res, *tid_res, escape_json_string(new_name),
      has_icon ? "true" : "false", icon_emoji_id.empty() ? "0" : icon_emoji_id);

  auto res = client_->send_request("editForumTopic", payload, 10.0);
  if (!res) {
    return std::unexpected("Failed to edit topic: " + res.error());
  }

  grm::log::info("Forum topic updated successfully.");
  return 0;
}

std::expected<int, std::string>
App::cmd_topic_toggle_close(const std::vector<std::string> &args, bool close) {
  if (args.size() < 2) {
    return std::unexpected("Usage: grm topic <close|reopen> <supergroup_id> <topic_id>");
  }

  auto cid_res = parse_int64(args[0]);
  auto tid_res = parse_int64(args[1]);
  if (!cid_res || !tid_res) return std::unexpected("Invalid supergroup_id or topic_id");

  if (auto res = ensure_authenticated(); !res) return std::unexpected(res.error());

  const std::string payload = std::format(
      R"({{
        "chat_id": {},
        "message_thread_id": {},
        "forum_topic_id": {},
        "is_closed": {}
      }})",
      *cid_res, *tid_res, *tid_res, close ? "true" : "false");

  auto res = client_->send_request("toggleForumTopicIsClosed", payload, 10.0);
  if (!res) {
    return std::unexpected("Failed to toggle topic closed state: " + res.error());
  }

  grm::log::info("Forum topic closed state updated successfully.");
  return 0;
}

std::expected<int, std::string>
App::cmd_topic_toggle_pin(const std::vector<std::string> &args, bool pin) {
  if (args.size() < 2) {
    return std::unexpected("Usage: grm topic <pin|unpin> <supergroup_id> <topic_id>");
  }

  auto cid_res = parse_int64(args[0]);
  auto tid_res = parse_int64(args[1]);
  if (!cid_res || !tid_res) return std::unexpected("Invalid supergroup_id or topic_id");

  if (auto res = ensure_authenticated(); !res) return std::unexpected(res.error());

  const std::string payload = std::format(
      R"({{
        "chat_id": {},
        "message_thread_id": {},
        "forum_topic_id": {},
        "is_pinned": {}
      }})",
      *cid_res, *tid_res, *tid_res, pin ? "true" : "false");

  auto res = client_->send_request("toggleForumTopicIsPinned", payload, 10.0);
  if (!res) {
    return std::unexpected("Failed to toggle topic pinned state: " + res.error());
  }

  grm::log::info("Forum topic pin state updated successfully.");
  return 0;
}


std::expected<int, std::string>
App::cmd_topic_delete(const std::vector<std::string> &args) {
  if (args.size() < 2) {
    return std::unexpected("Usage: grm topic delete <supergroup_id> <topic_id>");
  }

  auto cid_res = parse_int64(args[0]);
  auto tid_res = parse_int64(args[1]);
  if (!cid_res || !tid_res) return std::unexpected("Invalid supergroup_id or topic_id");

  if (auto res = ensure_authenticated(); !res) return std::unexpected(res.error());

  const std::string payload = std::format(
      R"({{
        "chat_id": {},
        "message_thread_id": {},
        "forum_topic_id": {}
      }})",
      *cid_res, *tid_res, *tid_res);

  auto res = client_->send_request("deleteForumTopic", payload, 10.0);
  if (!res) {
    return std::unexpected("Failed to delete topic: " + res.error());
  }

  grm::log::info("Forum topic deleted successfully.");
  return 0;
}



} // namespace grm
