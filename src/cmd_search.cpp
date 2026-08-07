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
  spec.description = "Universal cross-domain search across chats, supergroups, channels, users, messages, and files";

  spec.subcommands = {
      {"chats", "<query> [options]", "Search public and private chats, supergroups, and channels",
       {{"-n", "--limit", "<count>", "Maximum search results (default: 20)", {}},
        {"-o", "--offset", "<count>", "Offset starting result index (default: 0)", {}},
        {"-v", "--verbose", "", "Show verbose search metadata", {}}}},
      {"supergroups", "<query> [options]", "Search supergroups specifically (including forum supergroups)",
       {{"-n", "--limit", "<count>", "Maximum search results (default: 20)", {}},
        {"-o", "--offset", "<count>", "Offset starting result index (default: 0)", {}},
        {"-v", "--verbose", "", "Show verbose search metadata", {}}}},
      {"channels", "<query> [options]", "Search broadcast channels specifically",
       {{"-n", "--limit", "<count>", "Maximum search results (default: 20)", {}},
        {"-o", "--offset", "<count>", "Offset starting result index (default: 0)", {}},
        {"-v", "--verbose", "", "Show verbose search metadata", {}}}},
      {"msgs", "<query> [options]", "Search message history across all chats or scoped to a target chat",
       {{"-c", "--chat", "<chat_id>", "Restrict message search to target chat", {}},
        {"-n", "--limit", "<count>", "Maximum search results (default: 20)", {}},
        {"-o", "--offset", "<count>", "Offset starting result index (default: 0)", {}},
        {"-v", "--verbose", "", "Show verbose search metadata", {}}}},
      {"users", "<query> [options]", "Search contacts, DMs, and public user profiles",
       {{"-n", "--limit", "<count>", "Maximum search results (default: 20)", {}},
        {"-o", "--offset", "<count>", "Offset starting result index (default: 0)", {}},
        {"-v", "--verbose", "", "Show verbose search metadata", {}}}},
      {"files", "<query> [options]", "Search file and media attachments",
       {{"-t", "--type", "<doc|photo|video|audio|all>", "Filter attachment type (default: doc)", {"doc", "photo", "video", "audio", "all"}},
        {"-n", "--limit", "<count>", "Maximum search results (default: 20)", {}},
        {"-o", "--offset", "<count>", "Offset starting result index (default: 0)", {}},
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

struct SearchArgs {
  std::string query;
  int limit{20};
  int offset{0};
  int64_t chat_id{0};
  std::string type_filter{"doc"};
  bool verbose{false};
};

static SearchArgs parse_search_args(const std::vector<std::string> &args, int default_limit = 20) {
  SearchArgs result;
  result.limit = default_limit;

  for (size_t i = 0; i < args.size(); ++i) {
    const std::string &arg = args[i];

    if (arg == "-n" || arg == "--limit") {
      if (i + 1 < args.size()) {
        result.limit = std::atoi(args[++i].c_str());
      }
    } else if (arg.starts_with("--limit=")) {
      result.limit = std::atoi(arg.substr(8).c_str());
    } else if (arg.starts_with("-n=")) {
      result.limit = std::atoi(arg.substr(3).c_str());
    } else if (arg == "-o" || arg == "--offset") {
      if (i + 1 < args.size()) {
        result.offset = std::atoi(args[++i].c_str());
      }
    } else if (arg.starts_with("--offset=")) {
      result.offset = std::atoi(arg.substr(9).c_str());
    } else if (arg.starts_with("-o=")) {
      result.offset = std::atoi(arg.substr(3).c_str());
    } else if (arg == "-c" || arg == "--chat") {
      if (i + 1 < args.size()) {
        result.chat_id = std::stoll(args[++i]);
      }
    } else if (arg.starts_with("--chat=")) {
      result.chat_id = std::stoll(arg.substr(7));
    } else if (arg == "-t" || arg == "--type") {
      if (i + 1 < args.size()) {
        result.type_filter = args[++i];
      }
    } else if (arg.starts_with("--type=")) {
      result.type_filter = arg.substr(7);
    } else if (arg == "-v" || arg == "--verbose") {
      result.verbose = true;
    } else if (!arg.starts_with('-')) {
      if (result.query.empty()) {
        result.query = arg;
      }
    }
  }

  if (result.limit <= 0) result.limit = default_limit;
  if (result.offset < 0) result.offset = 0;
  return result;
}


std::expected<int, std::string>
App::cmd_search(const std::vector<std::string> &args) {
  if (args.empty() || is_help_requested(args)) {
    print_search_help(options_.format);
    return 0;
  }

  // Scan args for subcommand keyword
  std::string sub;
  std::vector<std::string> sub_args;
  for (size_t i = 0; i < args.size(); ++i) {
    const std::string &arg = args[i];
    if (sub.empty() && (arg == "chats" || arg == "supergroups" || arg == "supergroup" ||
                        arg == "groups" || arg == "channels" || arg == "channel" ||
                        arg == "msgs" || arg == "messages" ||
                        arg == "users" || arg == "contacts" || arg == "files")) {
      sub = arg;
    } else {
      sub_args.push_back(arg);
    }
  }

  if (sub == "chats") {
    return cmd_search_chats(sub_args);
  }
  if (sub == "supergroups" || sub == "supergroup" || sub == "groups") {
    return cmd_search_supergroups(sub_args);
  }
  if (sub == "channels" || sub == "channel") {
    return cmd_search_channels(sub_args);
  }
  if (sub == "msgs" || sub == "messages") {
    return cmd_search_msgs(sub_args);
  }
  if (sub == "users" || sub == "contacts") {
    return cmd_search_users(sub_args);
  }
  if (sub == "files") {
    return cmd_search_files(sub_args);
  }

  // Universal multi-domain query
  if (auto res = ensure_authenticated(); !res) {
    return std::unexpected(res.error());
  }

  SearchArgs sargs = parse_search_args(args, 10);
  if (sargs.query.empty()) {
    print_search_help(options_.format);
    return 0;
  }

  const std::string &query = sargs.query;
  int limit = sargs.limit;
  bool verbose = sargs.verbose || (options_.verbosity == log::VerbosityLevel::Verbose || options_.verbosity == log::VerbosityLevel::Debug);

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

struct ResolvedChatItems {
  std::vector<fmt::ChatItem> chats;
  std::vector<fmt::ChatItem> supergroups;
  std::vector<fmt::ChatItem> channels;
};

static std::vector<std::string> expand_search_query_variants(const std::string &query, int limit) {
  std::vector<std::string> variants;
  variants.push_back(query);

  std::string lower = query;
  std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);

  if (lower.ends_with('s') && lower.length() > 3) {
    std::string stem = lower.substr(0, lower.length() - 1);
    variants.push_back(stem);
  } else if (!lower.ends_with('s') && lower.length() >= 3) {
    variants.push_back(lower + "s");
  }

  if (lower == "bit") {
    variants.push_back("bitcoin");
    variants.push_back("bits");
    variants.push_back("btc");
  } else if (lower == "eth") {
    variants.push_back("ethereum");
    variants.push_back("ether");
  } else if (lower == "dev") {
    variants.push_back("developer");
    variants.push_back("development");
    variants.push_back("devops");
  } else if (lower == "doc") {
    variants.push_back("document");
    variants.push_back("documentation");
    variants.push_back("docs");
  }

  // Suffix/Prefix probes without spaces for global public handle/username directory search
  if (limit >= 20 && lower.find(' ') == std::string::npos && !lower.empty()) {
    variants.push_back(lower + "_");
    variants.push_back(lower + "bot");
    variants.push_back(lower + "hub");
    variants.push_back(lower + "link");
    variants.push_back(lower + "club");
    variants.push_back(lower + "hd");
    for (char c = 'a'; c <= 'z'; ++c) {
      variants.push_back(lower + c);
      if (static_cast<int>(variants.size()) >= std::min(limit, 35)) break;
    }
  }

  return variants;
}

static ResolvedChatItems resolve_chat_candidates(
    TdClient *client,
    const std::string &query,
    int limit,
    std::function<void(int64_t)> ensure_chat_loaded_fn) {

  (void)client->send_request("loadChats", R"({"limit": 500})", 1.0);

  const std::string escaped_query = escape_json_string(query);
  std::vector<int64_t> candidate_ids;

  // 1. Query searchChats (local database)
  const std::string req_local = std::format(R"({{"query": "{}", "limit": {}}})", escaped_query, limit * 3);
  if (auto res_local = client->send_request("searchChats", req_local, 2.0)) {
    for (const auto &id_val : res_local->get_array("chat_ids")) {
      if (auto id = id_val.as_int64()) {
        candidate_ids.push_back(*id);
      }
    }
  }

  // 2. Query searchChatsOnServer (server-side joined chat search)
  const std::string req_server = std::format(R"({{"query": "{}", "limit": {}}})", escaped_query, limit * 3);
  if (auto res_server = client->send_request("searchChatsOnServer", req_server, 2.0)) {
    for (const auto &id_val : res_server->get_array("chat_ids")) {
      if (auto id = id_val.as_int64()) {
        if (std::find(candidate_ids.begin(), candidate_ids.end(), *id) == candidate_ids.end()) {
          candidate_ids.push_back(*id);
        }
      }
    }
  }

  // 3. Scan getChats (all joined chats in memory) to guarantee joined matching chats are never missed
  if (auto res_get = client->send_request("getChats", R"({"limit": 300})", 1.0)) {
    std::string lower_q = query;
    std::transform(lower_q.begin(), lower_q.end(), lower_q.begin(), ::tolower);
    std::string stem = lower_q;
    if (stem.ends_with('s') && stem.length() > 3) {
      stem = stem.substr(0, stem.length() - 1);
    }

    for (const auto &id_val : res_get->get_array("chat_ids")) {
      if (auto id = id_val.as_int64()) {
        if (std::find(candidate_ids.begin(), candidate_ids.end(), *id) == candidate_ids.end()) {
          const std::string info_req = std::format(R"({{"chat_id": {}}})", *id);
          if (auto info_res = client->send_request("getChat", info_req, 0.1)) {
            std::string title = info_res->get_string("title").value_or("");
            std::string lower_title = title;
            std::transform(lower_title.begin(), lower_title.end(), lower_title.begin(), ::tolower);
            if (lower_title.find(lower_q) != std::string::npos || lower_title.find(stem) != std::string::npos) {
              candidate_ids.push_back(*id);
            }
          }
        }
      }
    }
  }

  // 4. Query searchPublicChats with query variants for global public chats/channels/supergroups
  std::vector<std::string> search_variants = expand_search_query_variants(query, limit);
  for (const auto &var : search_variants) {
    if (static_cast<int>(candidate_ids.size()) >= limit * 3) break;
    const std::string req_pub = std::format(R"({{"query": "{}"}})", escape_json_string(var));
    if (auto res_pub = client->send_request("searchPublicChats", req_pub, 0.15)) {
      for (const auto &id_val : res_pub->get_array("chat_ids")) {
        if (auto id = id_val.as_int64()) {
          if (std::find(candidate_ids.begin(), candidate_ids.end(), *id) == candidate_ids.end()) {
            candidate_ids.push_back(*id);
          }
        }
      }
    }
  }

  // 5. Query searchPublicChat if single word or handle provided
  std::string handle = query;
  if (handle.starts_with('@')) {
    handle = handle.substr(1);
  }
  if (!handle.empty() && handle.find(' ') == std::string::npos) {
    const std::string pub_chat_req = std::format(R"({{"username": "{}"}})", escape_json_string(handle));
    if (auto chat_res = client->send_request("searchPublicChat", pub_chat_req, 0.5)) {
      if (auto id = chat_res->get_int("id")) {
        if (std::find(candidate_ids.begin(), candidate_ids.end(), *id) == candidate_ids.end()) {
          candidate_ids.push_back(*id);
        }
      }
    }
  }


  // 5. Global Message Discovery: Paginate searchMessages to discover active chats & channels discussing query
  int64_t from_msg_id = 0;
  int64_t from_c_id = 0;
  for (int p = 0; p < 3; ++p) {
    if (static_cast<int>(candidate_ids.size()) >= limit * 3) break;
    const std::string msg_req = std::format(
        R"({{"query": "{}", "offset_chat_id": {}, "offset_message_id": {}, "limit": 100}})",
        escaped_query, from_c_id, from_msg_id);
    auto res_msg = client->send_request("searchMessages", msg_req, 1.5);
    if (!res_msg) break;

    auto msgs_arr = res_msg->get_array("messages");
    if (msgs_arr.empty()) break;

    for (const auto &m : msgs_arr) {
      if (auto cid = m.get_int("chat_id")) {
        if (std::find(candidate_ids.begin(), candidate_ids.end(), *cid) == candidate_ids.end()) {
          candidate_ids.push_back(*cid);
        }
      }

      std::string text;
      if (auto content = m.get_object("content")) {
        if (auto text_obj = content->get_object("text")) {
          text = text_obj->get_string("text").value_or("");
        } else if (auto caption_obj = content->get_object("caption")) {
          text = caption_obj->get_string("text").value_or("");
        }
      }

      // Extract @username handles mentioned in messages
      size_t pos = 0;
      while ((pos = text.find('@', pos)) != std::string::npos) {
        size_t end = pos + 1;
        while (end < text.length() && (std::isalnum(text[end]) || text[end] == '_')) {
          end++;
        }
        if (end - pos > 4 && end - pos < 33) {
          std::string h = text.substr(pos + 1, end - pos - 1);
          const std::string pub_h_req = std::format(R"({{"username": "{}"}})", escape_json_string(h));
          if (auto pub_res = client->send_request("searchPublicChat", pub_h_req, 0.2)) {
            if (auto pid = pub_res->get_int("id")) {
              if (std::find(candidate_ids.begin(), candidate_ids.end(), *pid) == candidate_ids.end()) {
                candidate_ids.push_back(*pid);
              }
            }
          }
        }
        pos = end;
      }

      from_msg_id = m.get_int("id").value_or(0);
      from_c_id = m.get_int("chat_id").value_or(0);
    }
  }


  ResolvedChatItems result;
  for (int64_t id : candidate_ids) {
    ensure_chat_loaded_fn(id);
    const std::string info_req = std::format(R"({{"chat_id": {}}})", id);
    if (auto info_res = client->send_request("getChat", info_req, 0.5)) {
      fmt::ChatItem item;
      item.id = id;
      item.title = info_res->get_string("title").value_or("Chat " + std::to_string(id));
      bool is_supergroup = false;
      bool is_channel = false;

      if (auto type_obj = info_res->get_object("type")) {
        std::string t = type_obj->get_string("@type").value_or("chat");
        if (t == "chatTypeSupergroup") {
          bool ch = type_obj->get_bool("is_channel").value_or(false);
          if (!ch) {
            is_supergroup = true;
            item.type = type_obj->get_bool("is_forum").value_or(false) ? "Forum Supergroup" : "Supergroup";
          } else {
            is_channel = true;
            item.type = "Channel";
          }
        } else if (t == "chatTypeBasicGroup") {
          item.type = "Basic Group";
        } else if (t == "chatTypePrivate") {
          item.type = "Private Chat";
        } else {
          item.type = t;
        }
      }
      item.unread_count = static_cast<int32_t>(info_res->get_int("unread_count").value_or(0));

      std::string lower_query = query;
      std::transform(lower_query.begin(), lower_query.end(), lower_query.begin(), ::tolower);

      std::string lower_title = item.title;
      std::transform(lower_title.begin(), lower_title.end(), lower_title.begin(), ::tolower);

      std::string stem = lower_query;
      if (stem.ends_with('s') && stem.length() > 3) {
        stem = stem.substr(0, stem.length() - 1);
      }

      if (lower_title.find(lower_query) != std::string::npos || lower_title.find(stem) != std::string::npos) {
        if (is_supergroup) {
          result.supergroups.push_back(item);
        } else if (is_channel) {
          result.channels.push_back(item);
        } else {
          result.chats.push_back(item);
        }
      }
    }
  }


  return result;
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

  SearchArgs sargs = parse_search_args(args, 20);
  if (sargs.query.empty()) {
    print_search_help(options_.format);
    return 0;
  }

  bool verbose = sargs.verbose || (options_.verbosity == log::VerbosityLevel::Verbose || options_.verbosity == log::VerbosityLevel::Debug);
  int needed = sargs.offset + sargs.limit;
  ResolvedChatItems res_items = resolve_chat_candidates(client_.get(), sargs.query, needed, [this](int64_t id) { ensure_chat_loaded(id); });

  std::vector<fmt::ChatItem> combined = res_items.chats;
  combined.insert(combined.end(), res_items.supergroups.begin(), res_items.supergroups.end());
  combined.insert(combined.end(), res_items.channels.begin(), res_items.channels.end());

  std::vector<fmt::ChatItem> sliced;
  if (static_cast<int>(combined.size()) > sargs.offset) {
    sliced = std::vector<fmt::ChatItem>(combined.begin() + sargs.offset, combined.end());
    if (static_cast<int>(sliced.size()) > sargs.limit) {
      sliced.resize(sargs.limit);
    }
  }

  fmt::Formatter::print_chats(sliced, options_.format, options_.color_mode, std::cout, verbose);
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

  SearchArgs sargs = parse_search_args(args, 20);
  if (sargs.query.empty()) {
    print_search_help(options_.format);
    return 0;
  }

  bool verbose = sargs.verbose || (options_.verbosity == log::VerbosityLevel::Verbose || options_.verbosity == log::VerbosityLevel::Debug);
  int needed = sargs.offset + sargs.limit;
  ResolvedChatItems res_items = resolve_chat_candidates(client_.get(), sargs.query, needed, [this](int64_t id) { ensure_chat_loaded(id); });

  std::vector<fmt::ChatItem> supergroup_list = res_items.supergroups;
  supergroup_list.insert(supergroup_list.end(), res_items.channels.begin(), res_items.channels.end());

  std::vector<fmt::ChatItem> sliced;
  if (static_cast<int>(supergroup_list.size()) > sargs.offset) {
    sliced = std::vector<fmt::ChatItem>(supergroup_list.begin() + sargs.offset, supergroup_list.end());
    if (static_cast<int>(sliced.size()) > sargs.limit) {
      sliced.resize(sargs.limit);
    }
  }

  fmt::Formatter::print_chats(sliced, options_.format, options_.color_mode, std::cout, verbose);
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

  SearchArgs sargs = parse_search_args(args, 20);
  if (sargs.query.empty()) {
    print_search_help(options_.format);
    return 0;
  }

  const std::string &query = sargs.query;
  int64_t chat_id = sargs.chat_id;
  int target_needed = sargs.offset + sargs.limit;
  bool verbose = sargs.verbose || (options_.verbosity == log::VerbosityLevel::Verbose || options_.verbosity == log::VerbosityLevel::Debug);

  std::vector<fmt::MessageItem> all_messages;
  int64_t from_message_id = 0;
  int64_t from_chat_id = 0;

  while (static_cast<int>(all_messages.size()) < target_needed) {
    std::string req;
    if (chat_id != 0) {
      req = std::format(
          R"({{"chat_id": {}, "query": "{}", "from_message_id": {}, "limit": 100}})",
          chat_id, escape_json_string(query), from_message_id);
    } else {
      req = std::format(
          R"({{"query": "{}", "offset_chat_id": {}, "offset_message_id": {}, "limit": 100}})",
          escape_json_string(query), from_chat_id, from_message_id);
    }

    auto call = client_->send_request(chat_id != 0 ? "searchChatMessages" : "searchMessages", req, 5.0);
    if (!call) break;

    auto msgs_arr = call->get_array("messages");
    if (msgs_arr.empty()) break;

    int new_msgs = 0;
    for (const auto &m : msgs_arr) {
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

      all_messages.push_back(item);
      from_message_id = item.id;
      from_chat_id = item.chat_id;
      new_msgs++;
    }

    if (new_msgs == 0) break;
  }

  std::vector<fmt::MessageItem> result_messages;
  if (static_cast<int>(all_messages.size()) > sargs.offset) {
    result_messages = std::vector<fmt::MessageItem>(
        all_messages.begin() + sargs.offset,
        all_messages.end());
    if (static_cast<int>(result_messages.size()) > sargs.limit) {
      result_messages.resize(sargs.limit);
    }
  }

  fmt::Formatter::print_messages(result_messages, options_.format, options_.color_mode, std::cout, verbose);
  return 0;
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

  SearchArgs sargs = parse_search_args(args, 20);
  if (sargs.query.empty()) {
    print_search_help(options_.format);
    return 0;
  }

  const std::string &query = sargs.query;
  int target_needed = sargs.offset + sargs.limit;
  bool verbose = sargs.verbose || (options_.verbosity == log::VerbosityLevel::Verbose || options_.verbosity == log::VerbosityLevel::Debug);

  // 1. Warm TDLib chat cache
  (void)client_->send_request("loadChats", R"({"limit": 500})", 0.5);

  std::vector<int64_t> candidate_user_ids;
  const std::string escaped_query = escape_json_string(query);

  // 2. Search local contacts
  const std::string req = std::format(R"({{"query": "{}", "limit": {}}})", escaped_query, target_needed * 2);
  if (auto res_contacts = client_->send_request("searchContacts", req, 1.5)) {
    for (const auto &user_val : res_contacts->get_array("users")) {
      if (auto id = user_val.as_int64()) {
        candidate_user_ids.push_back(*id);
      }
    }
  }

  // 3. Query searchChatsOnServer for private user chats
  const std::string req_server = std::format(R"({{"query": "{}", "limit": {}}})", escaped_query, target_needed * 2);
  if (auto res_server = client_->send_request("searchChatsOnServer", req_server, 1.5)) {
    for (const auto &id_val : res_server->get_array("chat_ids")) {
      if (auto cid = id_val.as_int64()) {
        const std::string info_req = std::format(R"({{"chat_id": {}}})", *cid);
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

  // 4. Query searchPublicChat if handle provided or single word
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

  // 5. Query searchPublicChats with query variants for public user profiles
  std::vector<std::string> user_variants = expand_search_query_variants(query, target_needed);
  for (const auto &var : user_variants) {
    if (static_cast<int>(candidate_user_ids.size()) >= target_needed * 2) break;
    const std::string pub_chats_req = std::format(R"({{"query": "{}"}})", escape_json_string(var));
    if (auto pub_chats_res = client_->send_request("searchPublicChats", pub_chats_req, 1.0)) {
      for (const auto &id_val : pub_chats_res->get_array("chat_ids")) {
        if (auto id = id_val.as_int64()) {
          const std::string info_req = std::format(R"({{"chat_id": {}}})", *id);
          if (auto info_res = client_->send_request("getChat", info_req, 0.3)) {
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
  }

  // 6. Global Message Sender Discovery: Extract user IDs from senders of messages matching query
  if (static_cast<int>(candidate_user_ids.size()) < target_needed) {
    const std::string msg_req = std::format(R"({{"query": "{}", "limit": {}}})", escaped_query, std::max(target_needed * 2, 50));
    if (auto res_msg = client_->send_request("searchMessages", msg_req, 2.0)) {
      for (const auto &m : res_msg->get_array("messages")) {
        if (auto sender_obj = m.get_object("sender_id")) {
          if (sender_obj->get_string("@type").value_or("") == "messageSenderUser") {
            if (auto uid = sender_obj->get_int("user_id")) {
              if (std::find(candidate_user_ids.begin(), candidate_user_ids.end(), *uid) == candidate_user_ids.end()) {
                candidate_user_ids.push_back(*uid);
              }
            }
          }
        }
      }
    }
  }

  // 7. Search members inside joined supergroups dynamically until target limit is reached
  if (static_cast<int>(candidate_user_ids.size()) < target_needed) {
    if (auto res_chats = client_->send_request("getChats", R"({"limit": 100})", 0.5)) {
      for (const auto &id_val : res_chats->get_array("chat_ids")) {
        if (static_cast<int>(candidate_user_ids.size()) >= target_needed * 2) break;
        if (auto cid = id_val.as_int64()) {
          const std::string info_req = std::format(R"({{"chat_id": {}}})", *cid);
          if (auto info_res = client_->send_request("getChat", info_req, 0.1)) {
            if (auto type_obj = info_res->get_object("type")) {
              if (type_obj->get_string("@type").value_or("") == "chatTypeSupergroup") {
                if (!type_obj->get_bool("is_channel").value_or(false)) {
                  const std::string mem_req = std::format(
                      R"({{"chat_id": {}, "query": "{}", "limit": {}}})",
                      *cid, escaped_query, target_needed);
                  if (auto mem_res = client_->send_request("searchChatMembers", mem_req, 0.2)) {
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
          }
        }
      }
    }
  }



  std::vector<fmt::UserItem> users;
  if (static_cast<int>(candidate_user_ids.size()) > sargs.offset) {
    std::vector<int64_t> sliced_ids(
        candidate_user_ids.begin() + sargs.offset,
        candidate_user_ids.end());
    if (static_cast<int>(sliced_ids.size()) > sargs.limit) {
      sliced_ids.resize(sargs.limit);
    }
    for (int64_t id : sliced_ids) {
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
      }
    }
  }

  fmt::Formatter::print_users(users, options_.format, options_.color_mode, std::cout, verbose);
  return 0;
}

std::expected<int, std::string>
App::cmd_search_channels(const std::vector<std::string> &args) {
  if (args.empty() || is_help_requested(args)) {
    print_search_help(options_.format);
    return 0;
  }

  if (auto res = ensure_authenticated(); !res) {
    return std::unexpected(res.error());
  }

  SearchArgs sargs = parse_search_args(args, 20);
  if (sargs.query.empty()) {
    print_search_help(options_.format);
    return 0;
  }

  bool verbose = sargs.verbose || (options_.verbosity == log::VerbosityLevel::Verbose || options_.verbosity == log::VerbosityLevel::Debug);
  int needed = sargs.offset + sargs.limit;
  ResolvedChatItems res_items = resolve_chat_candidates(client_.get(), sargs.query, needed, [this](int64_t id) { ensure_chat_loaded(id); });

  std::vector<fmt::ChatItem> sliced;
  if (static_cast<int>(res_items.channels.size()) > sargs.offset) {
    sliced = std::vector<fmt::ChatItem>(res_items.channels.begin() + sargs.offset, res_items.channels.end());
    if (static_cast<int>(sliced.size()) > sargs.limit) {
      sliced.resize(sargs.limit);
    }
  }

  fmt::Formatter::print_chats(sliced, options_.format, options_.color_mode, std::cout, verbose);
  return 0;
}

std::expected<int, std::string>
App::cmd_search_files(const std::vector<std::string> &args) {
  if (args.empty() || is_help_requested(args)) {
    print_search_help(options_.format);
    return 0;
  }

  if (auto res = ensure_authenticated(); !res) {
    return std::unexpected(res.error());
  }

  SearchArgs sargs = parse_search_args(args, 20);
  if (sargs.query.empty()) {
    print_search_help(options_.format);
    return 0;
  }

  const std::string &query = sargs.query;
  const std::string &type = sargs.type_filter;
  int target_needed = sargs.offset + sargs.limit;
  bool verbose = sargs.verbose || (options_.verbosity == log::VerbosityLevel::Verbose || options_.verbosity == log::VerbosityLevel::Debug);

  std::vector<std::string> filter_types;
  if (type == "photo") filter_types = {"searchMessagesFilterPhoto"};
  else if (type == "video") filter_types = {"searchMessagesFilterVideo"};
  else if (type == "audio") filter_types = {"searchMessagesFilterAudio"};
  else if (type == "all") filter_types = {"searchMessagesFilterDocument", "searchMessagesFilterPhoto", "searchMessagesFilterVideo", "searchMessagesFilterAudio"};
  else filter_types = {"searchMessagesFilterDocument"};

  std::vector<fmt::MessageItem> all_files;
  const std::string escaped_query = escape_json_string(query);

  for (const auto &ft : filter_types) {
    int64_t from_message_id = 0;
    int64_t from_chat_id = 0;

    while (static_cast<int>(all_files.size()) < target_needed) {
      const std::string file_req = std::format(
          R"({{"query": "{}", "filter": {{"@type": "{}"}}, "offset_chat_id": {}, "offset_message_id": {}, "limit": 100}})",
          escaped_query, ft, from_chat_id, from_message_id);
      auto file_res = client_->send_request("searchMessages", file_req, 5.0);
      if (!file_res) break;

      auto msgs_arr = file_res->get_array("messages");
      if (msgs_arr.empty()) break;

      int new_msgs = 0;
      for (const auto &m : msgs_arr) {
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
            item.attachment_type = ft.substr(20);
          }
          if (auto text_obj = content->get_object("caption")) {
            item.text = text_obj->get_string("text").value_or("");
          }
        }

        all_files.push_back(item);
        from_message_id = item.id;
        from_chat_id = item.chat_id;
        new_msgs++;
      }

      if (new_msgs == 0) break;
    }

    if (static_cast<int>(all_files.size()) >= target_needed) break;
  }

  std::vector<fmt::MessageItem> result_files;
  if (static_cast<int>(all_files.size()) > sargs.offset) {
    result_files = std::vector<fmt::MessageItem>(
        all_files.begin() + sargs.offset,
        all_files.end());
    if (static_cast<int>(result_files.size()) > sargs.limit) {
      result_files.resize(sargs.limit);
    }
  }

  fmt::Formatter::print_messages(result_files, options_.format, options_.color_mode, std::cout, verbose);
  return 0;
}

} // namespace grm


