#include "grm/app.hpp"
#include "grm/formatter.hpp"
#include "grm/json_utils.hpp"
#include "grm/list_options.hpp"
#include <charconv>
#include <format>
#include <iostream>
#include <regex>

namespace grm {

CommandSpec get_chat_spec() {
  return CommandSpec{
      "chat",
      "Manage Telegram chats, groups, and channels",
      {SubcommandSpec{
           "ls",
           "[-n|--limit <N>] [-S|--since <time>] [-f|--filter <pattern>] "
           "[-F|--folder <id>]",
           "List active conversations, groups, channels, and private chats",
           {OptionSpec{"-n",
                       "--limit",
                       "<N>",
                       "Maximum number of chats to display (default: 100)",
                       {}},
            OptionSpec{"-S",
                       "--since",
                       "<time>",
                       "Filter chats active since duration (e.g. '1 day ago')",
                       {}},
            OptionSpec{"-f",
                       "--filter",
                       "<pattern>",
                       "Filter chats by title or ID pattern filter",
                       {}},
            OptionSpec{"-F",
                       "--folder",
                       "<id>",
                       "Filter chat listing to specific chat folder ID",
                       {}},
            OptionSpec{"-h", "--help", "", "Show list help message", {}}}},

       SubcommandSpec{
           "create",
           "<group|channel> [--private|--public] \"<title>\"",
           "Create a new basic group, supergroup, or broadcast channel",
           {OptionSpec{
                "", "--private", "", "Create as a private chat/channel", {}},
            OptionSpec{
                "", "--public", "", "Create as a public chat/channel", {}},
            OptionSpec{"-h", "--help", "", "Show create help message", {}}}},
       SubcommandSpec{
           "info",
           "<chat_id>",
           "Display detailed chat or supergroup metadata",
           {OptionSpec{"-h", "--help", "", "Show info help message", {}}}},
       SubcommandSpec{
           "set-title",
           "<chat_id> \"<title>\"",
           "Update group or channel title",
           {OptionSpec{"-h", "--help", "", "Show set-title help message", {}}}},
       SubcommandSpec{
           "set-desc",
           "<chat_id> \"<description>\"",
           "Update group or channel description",
           {OptionSpec{"-h", "--help", "", "Show set-desc help message", {}}}},
       SubcommandSpec{
           "pin",
           "<chat_id>",
           "Pin chat to top of chat list",
           {OptionSpec{"-h", "--help", "", "Show pin help message", {}}}},
       SubcommandSpec{
           "unpin",
           "<chat_id>",
           "Unpin chat from chat list",
           {OptionSpec{"-h", "--help", "", "Show unpin help message", {}}}},
       SubcommandSpec{
           "delete",
           "<chat_id>",
           "Delete chat history or leave group/channel",
           {OptionSpec{"-h", "--help", "", "Show delete help message", {}}}}},
      {}};
}

static std::expected<int32_t, std::string> parse_int32(std::string_view str) {
  int32_t val = 0;
  auto [ptr, ec] = std::from_chars(str.data(), str.data() + str.size(), val);
  if (ec != std::errc{} || ptr != str.data() + str.size()) {
    return std::unexpected("Invalid integer: " + std::string(str));
  }
  return val;
}

static std::expected<int64_t, std::string> parse_int64(std::string_view str) {

  int64_t val = 0;
  auto [ptr, ec] = std::from_chars(str.data(), str.data() + str.size(), val);
  if (ec != std::errc{} || ptr != str.data() + str.size()) {
    return std::unexpected("Invalid integer: " + std::string(str));
  }
  return val;
}

std::expected<int, std::string>
App::cmd_chat_ls(const std::vector<std::string> &args) {
  if (is_help_requested(args)) {
    print_subcommand_help("chat", "ls");
    return 0;
  }

  std::vector<std::string> positionals;
  auto opts_res = ListOptions::parse(args, positionals);
  if (!opts_res) {
    print_subcommand_help("chat", "ls");
    return std::unexpected(opts_res.error());
  }
  const auto &opts = *opts_res;

  if (auto res = ensure_authenticated(); !res) {
    return std::unexpected(res.error());
  }

  std::string chat_query;
  if (opts.has_filter && !opts.filter_patterns.empty()) {
    chat_query = opts.filter_patterns[0];
  }

  std::vector<int64_t> target_chat_ids;
  if (!chat_query.empty()) {
    const std::string search_req = std::format(
        R"({{"query": "{}", "limit": 100}})", escape_json_string(chat_query));
    if (auto search_res =
            client_->send_request("searchChats", search_req, 5.0)) {
      auto found_ids = search_res->get_array("chat_ids");
      for (const auto &val : found_ids) {
        if (auto cid = val.as_int64()) {
          target_chat_ids.push_back(*cid);
        }
      }
    }
  }

  int32_t folder_id = -1;
  for (size_t i = 0; i < args.size(); ++i) {
    std::string_view arg(args[i]);
    if ((arg == "-F" || arg == "--folder") && i + 1 < args.size()) {
      if (auto fid = parse_int32(args[i + 1])) {
        folder_id = *fid;
      }
    } else if (arg.starts_with("--folder=")) {
      if (auto fid = parse_int32(std::string(arg.substr(9)))) {
        folder_id = *fid;
      }
    } else if (arg.starts_with("-F=")) {
      if (auto fid = parse_int32(std::string(arg.substr(3)))) {
        folder_id = *fid;
      }
    }
  }

  if (target_chat_ids.empty()) {
    std::string load_req =
        (folder_id >= 0)
            ? std::format(
                  R"({{"chat_list": {{"@type": "chatListFolder", "chat_folder_id": {}}}, "limit": 100}})",
                  folder_id)
            : R"({"limit": 100})";
    auto load_res = client_->send_request("loadChats", load_req, 5.0);
    if (!load_res) {
      grm::log::debug("loadChats: " + load_res.error());
    }

    std::string chats_req =
        (folder_id >= 0)
            ? std::format(
                  R"({{"chat_list": {{"@type": "chatListFolder", "chat_folder_id": {}}}, "limit": 100}})",
                  folder_id)
            : R"({"limit": 100})";
    auto chats_res = client_->send_request("getChats", chats_req, 10.0);
    if (!chats_res) {
      return std::unexpected("Failed to get chats: " + chats_res.error());
    }

    auto chat_ids = chats_res->get_array("chat_ids");
    for (const auto &id_val : chat_ids) {
      if (auto cid = id_val.as_int64()) {
        target_chat_ids.push_back(*cid);
      }
    }
  }

  std::vector<fmt::ChatItem> items;
  items.reserve(target_chat_ids.size());

  for (const int64_t cid : target_chat_ids) {
    const std::string chat_req = std::format(R"({{"chat_id": {}}})", cid);
    auto chat_info = client_->send_request("getChat", chat_req, 3.0);
    if (chat_info) {
      std::string title = chat_info->get_string("title").value_or("Private");
      std::string type_name = "Chat";
      if (auto type_obj = chat_info->get_object("type")) {
        type_name = type_obj->get_type().value_or("Chat");
      }
      int32_t unread =
          static_cast<int32_t>(chat_info->get_int("unread_count").value_or(0));
      int64_t last_date = 0;
      if (auto last_msg = chat_info->get_object("last_message")) {
        last_date = last_msg->get_int("date").value_or(0);
      }

      if (!opts.matches_since(last_date)) {
        continue;
      }

      if (!opts.matches_filter_multi({title, type_name, std::to_string(cid)})) {
        continue;
      }

      items.push_back(fmt::ChatItem{.id = cid,
                                    .type = type_name,
                                    .title = title,
                                    .unread_count = unread,
                                    .last_message_date = last_date});
      if (items.size() >= static_cast<size_t>(opts.limit)) {
        break;
      }
    }
  }

  if (opts.reverse_order) {
    std::reverse(items.begin(), items.end());
  }

  fmt::Formatter::render(items, "chat.ls", options_.format, options_.color_mode,
                         std::cout,
                         (options_.verbosity >= log::VerbosityLevel::Verbose));
  return 0;
}

std::expected<int, std::string>
App::cmd_chat_create(const std::vector<std::string> &args) {
  if (is_help_requested(args)) {
    print_subcommand_help("chat", "create");
    return 0;
  }

  if (args.size() < 2) {
    print_subcommand_help("chat", "create");
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
  if (is_help_requested(args)) {
    print_subcommand_help("chat", "info");
    return 0;
  }

  if (args.empty()) {
    print_subcommand_help("chat", "info");
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
  if (is_help_requested(args)) {
    print_subcommand_help("chat", "set-title");
    return 0;
  }

  if (args.size() < 2) {
    print_subcommand_help("chat", "set-title");
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
  if (is_help_requested(args)) {
    print_subcommand_help("chat", "set-desc");
    return 0;
  }

  if (args.size() < 2) {
    print_subcommand_help("chat", "set-desc");
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
App::cmd_chat_delete(const std::vector<std::string> &args) {
  if (is_help_requested(args)) {
    print_subcommand_help("chat", "delete");
    return 0;
  }

  if (args.empty()) {
    print_subcommand_help("chat", "delete");
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

std::expected<int, std::string>
App::cmd_chat_pin(const std::vector<std::string> &args) {
  if (is_help_requested(args)) {
    print_subcommand_help("chat", "pin");
    return 0;
  }

  if (args.empty()) {
    print_subcommand_help("chat", "pin");
    return std::unexpected("Usage: grm chat pin <chat_id>");
  }

  auto cid_res = parse_int64(args[0]);
  if (!cid_res)
    return std::unexpected(cid_res.error());

  if (auto res = ensure_authenticated(); !res)
    return std::unexpected(res.error());

  const std::string payload = std::format(
      R"({{
        "chat_list": {{"@type": "chatListMain"}},
        "chat_id": {},
        "is_pinned": true
      }})",
      *cid_res);

  auto res = client_->send_request("toggleChatIsPinned", payload, 5.0);
  if (!res) {
    return std::unexpected("Failed to pin chat: " + res.error());
  }

  grm::log::info("Chat " + std::to_string(*cid_res) + " pinned successfully.");
  return 0;
}

std::expected<int, std::string>
App::cmd_chat_unpin(const std::vector<std::string> &args) {
  if (is_help_requested(args)) {
    print_subcommand_help("chat", "unpin");
    return 0;
  }

  if (args.empty()) {
    print_subcommand_help("chat", "unpin");
    return std::unexpected("Usage: grm chat unpin <chat_id>");
  }

  auto cid_res = parse_int64(args[0]);
  if (!cid_res)
    return std::unexpected(cid_res.error());

  if (auto res = ensure_authenticated(); !res)
    return std::unexpected(res.error());

  const std::string payload = std::format(
      R"({{
        "chat_list": {{"@type": "chatListMain"}},
        "chat_id": {},
        "is_pinned": false
      }})",
      *cid_res);

  auto res = client_->send_request("toggleChatIsPinned", payload, 5.0);
  if (!res) {
    return std::unexpected("Failed to unpin chat: " + res.error());
  }

  grm::log::info("Chat " + std::to_string(*cid_res) +
                 " unpinned successfully.");
  return 0;
}

} // namespace grm
