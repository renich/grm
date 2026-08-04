#include "grm/app.hpp"
#include "grm/formatter.hpp"
#include "grm/json_utils.hpp"
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
App::cmd_chat_ls([[maybe_unused]] const std::vector<std::string> &args) {
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
        items.push_back(
            fmt::ChatItem{.id = *cid, .type = type_name, .title = title});
      }
    }
  }

  fmt::Formatter::print_chats(items, options_.format, options_.color_mode);
  return 0;
}

std::expected<int, std::string>
App::cmd_chat_create(const std::vector<std::string> &args) {
  if (args.size() < 2) {
    return std::unexpected(
        "Usage: grm chat create <group|channel> [--private|--public] \"<title>\"");
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
  if (!cid_res) return std::unexpected(cid_res.error());
  const int64_t chat_id = *cid_res;

  if (auto res = ensure_authenticated(); !res) return std::unexpected(res.error());
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
    return std::unexpected("Usage: grm chat set-title <chat_id> \"<new_title>\"");
  }

  auto cid_res = parse_int64(args[0]);
  if (!cid_res) return std::unexpected(cid_res.error());
  const int64_t chat_id = *cid_res;
  const std::string &title = args[1];

  if (auto res = ensure_authenticated(); !res) return std::unexpected(res.error());
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
    return std::unexpected("Usage: grm chat set-desc <chat_id> \"<description>\"");
  }

  auto cid_res = parse_int64(args[0]);
  if (!cid_res) return std::unexpected(cid_res.error());
  const int64_t chat_id = *cid_res;
  const std::string &desc = args[1];

  if (auto res = ensure_authenticated(); !res) return std::unexpected(res.error());
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
  if (!cid_res || !mid_res) return std::unexpected("Invalid chat_id or message_id");

  if (auto res = ensure_authenticated(); !res) return std::unexpected(res.error());

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
  if (!cid_res) return std::unexpected(cid_res.error());
  int64_t message_id = (args.size() > 1) ? parse_int64(args[1]).value_or(0) : 0;

  if (auto res = ensure_authenticated(); !res) return std::unexpected(res.error());

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
  if (!cid_res) return std::unexpected(cid_res.error());

  if (auto res = ensure_authenticated(); !res) return std::unexpected(res.error());

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
