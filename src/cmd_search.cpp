#include "grm/app.hpp"
#include "grm/command_registry.hpp"
#include "grm/formatter.hpp"
#include "grm/list_options.hpp"
#include <chrono>
#include <format>
#include <iostream>
#include <thread>

namespace grm {

CommandSpec App::get_search_spec() {
  CommandSpec spec;
  spec.name = "search";
  spec.description = "Universal cross-domain search across chats, supergroups, messages, and contacts";

  spec.subcommands = {
      {"chats", "<query> [options]", "Search public and private chats, supergroups, and channels",
       {{"-n", "--limit", "<count>", "Maximum search results (default: 20)", {}},
        {"-v", "--verbose", "", "Show verbose search metadata", {}}}},
      {"supergroups", "<query> [options]", "Search supergroups specifically (including forum supergroups)",
       {{"-n", "--limit", "<count>", "Maximum search results (default: 20)", {}},
        {"-v", "--verbose", "", "Show verbose search metadata", {}}}},
      {"msgs", "<query> [options]", "Search message history across all chats or scoped to a target chat",
       {{"-c", "--chat", "<chat_id>", "Restrict message search to target chat", {}},
        {"-n", "--limit", "<count>", "Maximum search results (default: 20)", {}},
        {"-v", "--verbose", "", "Show verbose search metadata", {}}}},
      {"users", "<query> [options]", "Search contacts and public user profiles",
       {{"-n", "--limit", "<count>", "Maximum search results (default: 20)", {}},
        {"-v", "--verbose", "", "Show verbose search metadata", {}}}}
  };

  return spec;
}

void App::print_search_help(fmt::OutputFormat format) {
  if (format == fmt::OutputFormat::Json || format == fmt::OutputFormat::JsonL) {
    std::cout << CommandRegistry::get_instance().render_command_help_json("search", false) << "\n";
  } else {
    std::cout << CommandRegistry::get_instance().render_command_help("search") << "\n";
  }
}

std::expected<int, std::string>
App::cmd_search(const std::vector<std::string> &args) {
  if (args.empty() || is_help_requested(args)) {
    print_search_help(options_.format);
    return 0;
  }

  const std::string &sub = args[0];
  std::vector<std::string> sub_args(args.begin() + 1, args.end());

  if (sub == "chats") {
    return cmd_search_chats(sub_args);
  }
  if (sub == "supergroups" || sub == "supergroup" || sub == "groups") {
    return cmd_search_supergroups(sub_args);
  }
  if (sub == "msgs" || sub == "messages") {
    return cmd_search_msgs(sub_args);
  }
  if (sub == "users" || sub == "contacts") {
    return cmd_search_users(sub_args);
  }


  // Universal multi-domain query
  if (auto res = ensure_authenticated(); !res) {
    return std::unexpected(res.error());
  }

  const std::string query = sub;
  int limit = 10;
  bool verbose = (options_.verbosity == log::VerbosityLevel::Verbose || options_.verbosity == log::VerbosityLevel::Debug);

  for (size_t i = 0; i < sub_args.size(); ++i) {
    if (sub_args[i] == "-n" || sub_args[i] == "--limit") {
      if (i + 1 < sub_args.size()) {
        limit = std::atoi(sub_args[++i].c_str());
      }
    } else if (sub_args[i] == "-v" || sub_args[i] == "--verbose") {
      verbose = true;
    }
  }

  fmt::SearchSummary summary;
  summary.query = query;

  // 1. Search chats & supergroups
  std::vector<int64_t> candidate_chat_ids;
  const std::string chat_req = std::format(R"({{"query": "{}", "limit": {}}})", query, limit * 2);
  if (auto chat_res = client_->send_request("searchChats", chat_req, 5.0)) {
    for (const auto &id_val : chat_res->get_array("chat_ids")) {
      if (auto id = id_val.as_int64()) {
        candidate_chat_ids.push_back(*id);
      }
    }
  }
  const std::string pub_req = std::format(R"({{"query": "{}"}})", query);
  if (auto pub_res = client_->send_request("searchPublicChats", pub_req, 5.0)) {
    for (const auto &id_val : pub_res->get_array("chat_ids")) {
      if (auto id = id_val.as_int64()) {
        if (std::find(candidate_chat_ids.begin(), candidate_chat_ids.end(), *id) == candidate_chat_ids.end()) {
          candidate_chat_ids.push_back(*id);
        }
      }
    }
  }

  for (int64_t id : candidate_chat_ids) {
    ensure_chat_loaded(id);
    const std::string info_req = std::format(R"({{"chat_id": {}}})", id);
    if (auto info_res = client_->send_request("getChat", info_req, 3.0)) {
      fmt::ChatItem item;
      item.id = id;
      item.title = info_res->get_string("title").value_or("Chat " + std::to_string(id));
      std::string type_name = "chat";
      bool is_supergroup = false;

      if (auto type_obj = info_res->get_object("type")) {
        type_name = type_obj->get_string("@type").value_or("chat");
        if (type_name == "chatTypeSupergroup") {
          bool is_channel = type_obj->get_bool("is_channel").value_or(false);
          if (!is_channel) {
            is_supergroup = true;
            item.type = type_obj->get_bool("is_forum").value_or(false) ? "Forum Supergroup" : "Supergroup";
          } else {
            item.type = "Channel";
          }
        } else if (type_name == "chatTypeBasicGroup") {
          item.type = "Basic Group";
        } else if (type_name == "chatTypePrivate") {
          item.type = "Private Chat";
        }
      }
      item.unread_count = static_cast<int32_t>(info_res->get_int("unread_count").value_or(0));

      if (is_supergroup) {
        if (static_cast<int>(summary.supergroups.size()) < limit) {
          summary.supergroups.push_back(item);
        }
      } else {
        if (static_cast<int>(summary.chats.size()) < limit) {
          summary.chats.push_back(item);
        }
      }
    }
  }

  // 2. Search users & contacts
  const std::string user_req = std::format(R"({{"query": "{}", "limit": {}}})", query, limit);
  if (auto user_res = client_->send_request("searchContacts", user_req, 5.0)) {
    for (const auto &user_val : user_res->get_array("users")) {
      if (auto uid = user_val.as_int64()) {
        const std::string info_req = std::format(R"({{"user_id": {}}})", *uid);
        if (auto u = client_->send_request("getUser", info_req, 3.0)) {
          fmt::UserItem item;
          item.id = *uid;
          item.first_name = u->get_string("first_name").value_or("");
          item.last_name = u->get_string("last_name").value_or("");
          item.username = u->get_string("username").value_or("");
          item.phone_number = u->get_string("phone_number").value_or("");
          summary.users.push_back(item);
        }
      }
    }
  }

  // 3. Search messages
  const std::string msg_req = std::format(R"({{"query": "{}", "limit": {}}})", query, limit);
  if (auto msg_res = client_->send_request("searchMessages", msg_req, 5.0)) {
    for (const auto &m : msg_res->get_array("messages")) {
      fmt::MessageItem item;
      item.id = m.get_int("id").value_or(0);
      item.chat_id = m.get_int("chat_id").value_or(0);
      item.date = m.get_int("date").value_or(0);
      item.sender = resolve_sender_name(m);

      if (auto content = m.get_object("content")) {
        if (auto text_obj = content->get_object("text")) {
          item.text = text_obj->get_string("text").value_or("");
        } else if (auto caption_obj = content->get_object("caption")) {
          item.text = caption_obj->get_string("text").value_or("");
        }
      }

      summary.messages.push_back(item);
    }
  }

  // 4. Search files & document attachments
  const std::string file_req = std::format(R"({{"query": "{}", "filter": {{"@type": "searchMessagesFilterDocument"}}, "limit": {}}})", query, limit);
  if (auto file_res = client_->send_request("searchMessages", file_req, 5.0)) {
    for (const auto &m : file_res->get_array("messages")) {
      fmt::MessageItem item;
      item.id = m.get_int("id").value_or(0);
      item.chat_id = m.get_int("chat_id").value_or(0);
      item.date = m.get_int("date").value_or(0);
      item.sender = resolve_sender_name(m);
      item.has_attachment = true;

      if (auto content = m.get_object("content")) {
        if (auto doc_obj = content->get_object("document")) {
          item.attachment_type = doc_obj->get_string("file_name").value_or("Document");
        } else {
          item.attachment_type = "Document";
        }
        if (auto text_obj = content->get_object("caption")) {
          item.text = text_obj->get_string("text").value_or("");
        }
      }

      summary.files.push_back(item);
    }
  }

  fmt::Formatter::print_search_summary(summary, options_.format, options_.color_mode, std::cout, verbose);
  return 0;

}

std::expected<int, std::string>
App::cmd_search_chats(const std::vector<std::string> &args) {
  if (args.empty() || is_help_requested(args)) {
    print_search_help(options_.format);
    return 0;
  }

  if (auto res = ensure_authenticated(); !res) {
    return std::unexpected(res.error());
  }

  const std::string &query = args[0];
  int limit = 20;
  bool verbose = (options_.verbosity == log::VerbosityLevel::Verbose || options_.verbosity == log::VerbosityLevel::Debug);

  for (size_t i = 1; i < args.size(); ++i) {
    if (args[i] == "-n" || args[i] == "--limit") {
      if (i + 1 < args.size()) {
        limit = std::atoi(args[++i].c_str());
      }
    } else if (args[i] == "-v" || args[i] == "--verbose") {
      verbose = true;
    }
  }

  // 1. Warm TDLib memory cache
  (void)client_->send_request("loadChats", R"({"limit": 100})", 3.0);

  const std::string escaped_query = escape_json_string(query);
  std::vector<int64_t> candidate_ids;

  // 2. Query searchChats (local database)
  const std::string req_local = std::format(R"({{"query": "{}", "limit": {}}})", escaped_query, limit * 2);
  if (auto res_local = client_->send_request("searchChats", req_local, 5.0)) {
    for (const auto &id_val : res_local->get_array("chat_ids")) {
      if (auto id = id_val.as_int64()) {
        candidate_ids.push_back(*id);
      }
    }
  }

  // 3. Query searchChatsOnServer (server-side joined chat search)
  const std::string req_server = std::format(R"({{"query": "{}", "limit": {}}})", escaped_query, limit * 2);
  if (auto res_server = client_->send_request("searchChatsOnServer", req_server, 5.0)) {
    for (const auto &id_val : res_server->get_array("chat_ids")) {
      if (auto id = id_val.as_int64()) {
        if (std::find(candidate_ids.begin(), candidate_ids.end(), *id) == candidate_ids.end()) {
          candidate_ids.push_back(*id);
        }
      }
    }
  }

  // 4. Query searchPublicChats (global public chats/channels/supergroups)
  const std::string req_pub = std::format(R"({{"query": "{}"}})", escaped_query);
  if (auto res_pub = client_->send_request("searchPublicChats", req_pub, 5.0)) {
    for (const auto &id_val : res_pub->get_array("chat_ids")) {
      if (auto id = id_val.as_int64()) {
        if (std::find(candidate_ids.begin(), candidate_ids.end(), *id) == candidate_ids.end()) {
          candidate_ids.push_back(*id);
        }
      }
    }
  }

  std::vector<fmt::ChatItem> chats;
  for (int64_t id : candidate_ids) {
    ensure_chat_loaded(id);
    const std::string info_req = std::format(R"({{"chat_id": {}}})", id);
    if (auto info_res = client_->send_request("getChat", info_req, 3.0)) {
      fmt::ChatItem item;
      item.id = id;
      item.title = info_res->get_string("title").value_or("Chat " + std::to_string(id));
      if (auto type_obj = info_res->get_object("type")) {
        std::string t = type_obj->get_string("@type").value_or("chat");
        if (t == "chatTypeSupergroup") {
          item.type = type_obj->get_bool("is_channel").value_or(false) ? "Channel" : "Supergroup";
        } else if (t == "chatTypeBasicGroup") {
          item.type = "Basic Group";
        } else if (t == "chatTypePrivate") {
          item.type = "Private Chat";
        } else {
          item.type = t;
        }
      }
      item.unread_count = static_cast<int32_t>(info_res->get_int("unread_count").value_or(0));
      chats.push_back(item);
      if (static_cast<int>(chats.size()) >= limit) break;
    }
  }

  fmt::Formatter::print_chats(chats, options_.format, options_.color_mode, std::cout, verbose);
  return 0;
}


std::expected<int, std::string>
App::cmd_search_msgs(const std::vector<std::string> &args) {
  if (args.empty() || is_help_requested(args)) {
    print_search_help(options_.format);
    return 0;
  }

  if (auto res = ensure_authenticated(); !res) {
    return std::unexpected(res.error());
  }

  const std::string &query = args[0];
  int64_t chat_id = 0;
  int limit = 20;
  bool verbose = (options_.verbosity == log::VerbosityLevel::Verbose || options_.verbosity == log::VerbosityLevel::Debug);

  for (size_t i = 1; i < args.size(); ++i) {
    if (args[i] == "-c" || args[i] == "--chat") {
      if (i + 1 < args.size()) {
        chat_id = std::stoll(args[++i]);
      }
    } else if (args[i] == "-n" || args[i] == "--limit") {
      if (i + 1 < args.size()) {
        limit = std::atoi(args[++i].c_str());
      }
    } else if (args[i] == "-v" || args[i] == "--verbose") {
      verbose = true;
    }
  }

  std::optional<JsonValue> res;
  if (chat_id != 0) {
    ensure_chat_loaded(chat_id);
    const std::string req = std::format(R"({{"chat_id": {}, "query": "{}", "limit": {}}})", chat_id, query, limit);
    if (auto call = client_->send_request("searchChatMessages", req, 5.0)) {
      res = *call;
    }
  } else {
    const std::string req = std::format(R"({{"query": "{}", "limit": {}}})", query, limit);
    if (auto call = client_->send_request("searchMessages", req, 5.0)) {
      res = *call;
    }
  }

  if (!res) {
    return std::unexpected("Failed to search messages");
  }

  std::vector<fmt::MessageItem> messages;
  for (const auto &m : res->get_array("messages")) {
    fmt::MessageItem item;
    item.id = m.get_int("id").value_or(0);
    item.chat_id = m.get_int("chat_id").value_or(0);
    item.date = m.get_int("date").value_or(0);
    item.sender = resolve_sender_name(m);

    if (auto content = m.get_object("content")) {
      if (auto text_obj = content->get_object("text")) {
        item.text = text_obj->get_string("text").value_or("");
      } else if (auto caption_obj = content->get_object("caption")) {
        item.text = caption_obj->get_string("text").value_or("");
      }
    }

    messages.push_back(item);
  }

  fmt::Formatter::print_messages(messages, options_.format, options_.color_mode, std::cout, verbose);
  return 0;
}

static int parse_limit_option(const std::vector<std::string> &args, int default_limit) {
  int limit = default_limit;
  for (size_t i = 0; i < args.size(); ++i) {
    const std::string &arg = args[i];
    if (arg == "-n" || arg == "--limit") {
      if (i + 1 < args.size()) {
        limit = std::atoi(args[++i].c_str());
      }
    } else if (arg.starts_with("--limit=")) {
      limit = std::atoi(arg.substr(8).c_str());
    } else if (arg.starts_with("-n=")) {
      limit = std::atoi(arg.substr(3).c_str());
    }
  }
  return limit > 0 ? limit : default_limit;
}

std::expected<int, std::string>
App::cmd_search_users(const std::vector<std::string> &args) {
  if (args.empty() || is_help_requested(args)) {
    print_search_help(options_.format);
    return 0;
  }

  if (auto res = ensure_authenticated(); !res) {
    return std::unexpected(res.error());
  }

  const std::string &query = args[0];
  int limit = parse_limit_option(args, 20);
  bool verbose = (options_.verbosity == log::VerbosityLevel::Verbose || options_.verbosity == log::VerbosityLevel::Debug);

  for (size_t i = 1; i < args.size(); ++i) {
    if (args[i] == "-v" || args[i] == "--verbose") {
      verbose = true;
    }
  }

  std::vector<int64_t> candidate_user_ids;

  // 1. Search local contacts
  const std::string escaped_query = escape_json_string(query);
  const std::string req = std::format(R"({{"query": "{}", "limit": {}}})", escaped_query, limit);
  if (auto res_contacts = client_->send_request("searchContacts", req, 5.0)) {
    for (const auto &user_val : res_contacts->get_array("users")) {
      if (auto id = user_val.as_int64()) {
        candidate_user_ids.push_back(*id);
      }
    }
  }

  // 2. Query searchPublicChat if handle provided or single word
  std::string handle = query;
  if (handle.starts_with('@')) {
    handle = handle.substr(1);
  }
  if (!handle.empty() && handle.find(' ') == std::string::npos) {
    const std::string pub_user_req = std::format(R"({{"username": "{}"}})", escape_json_string(handle));
    if (auto chat_res = client_->send_request("searchPublicChat", pub_user_req, 3.0)) {
      if (auto type_obj = chat_res->get_object("type")) {
        if (type_obj->get_string("@type").value_or("") == "chatTypePrivate") {
          if (auto uid = type_obj->get_int("user_id")) {
            if (std::find(candidate_user_ids.begin(), candidate_user_ids.end(), *uid) == candidate_user_ids.end()) {
              candidate_user_ids.push_back(*uid);
            }
          }
        }
      }
    }
  }

  // 3. Query searchPublicChats for user profiles
  const std::string pub_chats_req = std::format(R"({{"query": "{}"}})", escaped_query);
  if (auto pub_chats_res = client_->send_request("searchPublicChats", pub_chats_req, 3.0)) {
    for (const auto &id_val : pub_chats_res->get_array("chat_ids")) {
      if (auto id = id_val.as_int64()) {
        const std::string info_req = std::format(R"({{"chat_id": {}}})", *id);
        if (auto info_res = client_->send_request("getChat", info_req, 3.0)) {
          if (auto type_obj = info_res->get_object("type")) {
            if (type_obj->get_string("@type").value_or("") == "chatTypePrivate") {
              if (auto uid = type_obj->get_int("user_id")) {
                if (std::find(candidate_user_ids.begin(), candidate_user_ids.end(), *uid) == candidate_user_ids.end()) {
                  candidate_user_ids.push_back(*uid);
                }
              }
            }
          }
        }
      }
    }
  }

  // 4. Search members inside joined supergroups (searchChatMembers)
  if (auto res_chats = client_->send_request("searchChats", R"({"limit": 50})", 3.0)) {
    for (const auto &id_val : res_chats->get_array("chat_ids")) {
      if (auto cid = id_val.as_int64()) {
        const std::string mem_req = std::format(
            R"({{"chat_id": {}, "query": "{}", "limit": {}}})",
            *cid, escaped_query, limit);
        if (auto mem_res = client_->send_request("searchChatMembers", mem_req, 2.0)) {
          for (const auto &m : mem_res->get_array("members")) {
            if (auto member_id = m.get_object("member_id")) {
              if (member_id->get_string("@type").value_or("") == "messageSenderUser") {
                if (auto uid = member_id->get_int("user_id")) {
                  if (std::find(candidate_user_ids.begin(), candidate_user_ids.end(), *uid) == candidate_user_ids.end()) {
                    candidate_user_ids.push_back(*uid);
                  }
                }
              }
            }
          }
        }
      }
    }
  }

  std::vector<fmt::UserItem> users;
  for (int64_t id : candidate_user_ids) {
    const std::string info_req = std::format(R"({{"user_id": {}}})", id);
    if (auto u = client_->send_request("getUser", info_req, 3.0)) {
      fmt::UserItem item;
      item.id = id;
      item.first_name = u->get_string("first_name").value_or("");
      item.last_name = u->get_string("last_name").value_or("");
      item.username = u->get_string("username").value_or("");
      item.phone_number = u->get_string("phone_number").value_or("");

      if (auto status_obj = u->get_object("status")) {
        item.status = status_obj->get_string("@type").value_or("Unknown");
      }

      users.push_back(item);
      if (static_cast<int>(users.size()) >= limit) break;
    }
  }

  fmt::Formatter::print_users(users, options_.format, options_.color_mode, std::cout, verbose);
  return 0;
}



std::expected<int, std::string>
App::cmd_search_supergroups(const std::vector<std::string> &args) {
  if (args.empty() || is_help_requested(args)) {
    print_search_help(options_.format);
    return 0;
  }

  if (auto res = ensure_authenticated(); !res) {
    return std::unexpected(res.error());
  }

  const std::string &query = args[0];
  int limit = 20;
  bool verbose = (options_.verbosity == log::VerbosityLevel::Verbose || options_.verbosity == log::VerbosityLevel::Debug);

  for (size_t i = 1; i < args.size(); ++i) {
    if (args[i] == "-n" || args[i] == "--limit") {
      if (i + 1 < args.size()) {
        limit = std::atoi(args[++i].c_str());
      }
    } else if (args[i] == "-v" || args[i] == "--verbose") {
      verbose = true;
    }
  }

  std::vector<int64_t> candidate_chat_ids;

  // Search local/joined chats
  const std::string local_req = std::format(R"({{"query": "{}", "limit": {}}})", query, limit * 2);
  if (auto res = client_->send_request("searchChats", local_req, 5.0)) {
    for (const auto &id_val : res->get_array("chat_ids")) {
      if (auto id = id_val.as_int64()) {
        candidate_chat_ids.push_back(*id);
      }
    }
  }

  // Search public global chats
  const std::string pub_req = std::format(R"({{"query": "{}"}})", query);
  if (auto res = client_->send_request("searchPublicChats", pub_req, 5.0)) {
    for (const auto &id_val : res->get_array("chat_ids")) {
      if (auto id = id_val.as_int64()) {
        if (std::find(candidate_chat_ids.begin(), candidate_chat_ids.end(), *id) == candidate_chat_ids.end()) {
          candidate_chat_ids.push_back(*id);
        }
      }
    }
  }

  std::vector<fmt::ChatItem> supergroups;
  for (int64_t id : candidate_chat_ids) {
    ensure_chat_loaded(id);
    const std::string info_req = std::format(R"({{"chat_id": {}}})", id);
    if (auto info_res = client_->send_request("getChat", info_req, 3.0)) {
      if (auto type_obj = info_res->get_object("type")) {
        std::string type_name = type_obj->get_string("@type").value_or("");
        if (type_name == "chatTypeSupergroup") {
          bool is_channel = type_obj->get_bool("is_channel").value_or(false);
          if (!is_channel) {
            fmt::ChatItem item;
            item.id = id;
            item.title = info_res->get_string("title").value_or("Supergroup " + std::to_string(id));
            item.type = type_obj->get_bool("is_forum").value_or(false) ? "Forum Supergroup" : "Supergroup";
            item.unread_count = static_cast<int32_t>(info_res->get_int("unread_count").value_or(0));
            supergroups.push_back(item);
            if (static_cast<int>(supergroups.size()) >= limit) break;
          }
        }
      }
    }
  }

  fmt::Formatter::print_chats(supergroups, options_.format, options_.color_mode, std::cout, verbose);
  return 0;
}

} // namespace grm

