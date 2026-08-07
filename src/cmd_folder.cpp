#include "grm/app.hpp"
#include "grm/formatter.hpp"
#include "grm/json_utils.hpp"
#include "grm/logger.hpp"
#include <charconv>
#include <format>
#include <iostream>

namespace grm {

CommandSpec get_folder_spec() {
  return CommandSpec{
      "folder",
      "Manage Telegram chat folders and organization filters",
      {
          SubcommandSpec{"ls", "[-v|--verbose]", "List all configured chat folders and organization filters", {
              OptionSpec{"-v", "--verbose", "", "Display verbose folder details and chat ID lists", {}},
              OptionSpec{"-h", "--help", "", "Show folder list help message", {}}
          }},
          SubcommandSpec{"create", "<title> [options...]", "Create a new chat folder filter", {
              OptionSpec{"-g", "--include-groups", "", "Include group chats in folder", {}},
              OptionSpec{"-c", "--include-channels", "", "Include channels in folder", {}},
              OptionSpec{"-b", "--include-bots", "", "Include bots in folder", {}},
              OptionSpec{"-C", "--include-contacts", "", "Include contacts in folder", {}},
              OptionSpec{"-N", "--include-non-contacts", "", "Include non-contacts in folder", {}},
              OptionSpec{"", "--exclude-muted", "", "Exclude muted chats", {}},
              OptionSpec{"", "--exclude-read", "", "Exclude read chats", {}},
              OptionSpec{"", "--exclude-archived", "", "Exclude archived chats", {}},
              OptionSpec{"-i", "--include-chats", "<ids...>", "Comma-separated chat IDs to include", {}},
              OptionSpec{"-p", "--pinned-chats", "<ids...>", "Comma-separated chat IDs to pin", {}},
              OptionSpec{"-h", "--help", "", "Show folder create help message", {}}
          }},
          SubcommandSpec{"edit", "<folder_id> [options...]", "Modify an existing chat folder title, filters, or chats", {
              OptionSpec{"-t", "--title", "<title>", "Update folder title", {}},
              OptionSpec{"-a", "--add-chat", "<id>", "Add chat ID to included list", {}},
              OptionSpec{"-r", "--remove-chat", "<id>", "Remove chat ID from included list", {}},
              OptionSpec{"-P", "--pin-chat", "<id>", "Pin chat ID in folder", {}},
              OptionSpec{"-U", "--unpin-chat", "<id>", "Unpin chat ID in folder", {}},
              OptionSpec{"-h", "--help", "", "Show folder edit help message", {}}
          }},
          SubcommandSpec{"delete", "<folder_id>", "Remove a chat folder filter by ID", {
              OptionSpec{"-h", "--help", "", "Show folder delete help message", {}}
          }}
      },
      {}
  };
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
    return std::unexpected("Invalid chat ID integer: " + std::string(str));
  }
  return val;
}

void App::print_folder_help(fmt::OutputFormat format) {
  if (format == fmt::OutputFormat::Json || format == fmt::OutputFormat::JsonL) {
    std::cout << CommandRegistry::get_instance().render_command_help_json("folder", false) << "\n";
  } else {
    std::cout << CommandRegistry::get_instance().render_command_help("folder") << "\n";
  }
}

std::expected<int, std::string>
App::cmd_folder(const std::vector<std::string> &args) {
  if (args.empty()) {
    return cmd_folder_ls({});
  }

  std::string sub = args[0];
  std::vector<std::string> sub_args(args.begin() + 1, args.end());

  if (sub == "ls" || sub == "list") {
    return cmd_folder_ls(sub_args);
  } else if (sub == "create") {
    return cmd_folder_create(sub_args);
  } else if (sub == "edit") {
    return cmd_folder_edit(sub_args);
  } else if (sub == "delete" || sub == "rm") {
    return cmd_folder_delete(sub_args);
  } else if (sub == "-h" || sub == "--help") {
    print_folder_help(options_.format);
    return 0;
  }

  return std::unexpected("Unknown folder subcommand: " + sub);
}

static std::string extract_folder_title(const JsonValue &item) {
  if (auto name_obj = item.get_object("name")) {
    if (auto text_obj = name_obj->get_object("text")) {
      if (auto t = text_obj->get_string("text")) {
        return *t;
      }
    }
  }
  return item.get_string("title").value_or("");
}

std::expected<int, std::string>
App::cmd_folder_ls(const std::vector<std::string> &args) {
  bool verbose = (options_.verbosity == log::VerbosityLevel::Verbose || options_.verbosity == log::VerbosityLevel::Debug);
  for (const auto &arg : args) {
    if (arg == "-v" || arg == "--verbose") {
      verbose = true;
    } else if (arg == "-h" || arg == "--help") {
      print_folder_help(options_.format);
      return 0;
    }
  }

  if (auto res = ensure_authenticated(); !res) {
    return std::unexpected(res.error());
  }

  auto folders_arr = client_->get_cached_chat_folders();
  if (folders_arr.empty()) {
    (void)client_->send_request("loadChats", R"({"limit": 100})", 3.0);
    for (int i = 0; i < 5; ++i) {
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
      folders_arr = client_->get_cached_chat_folders();
      if (!folders_arr.empty()) break;
    }
  }

  std::vector<fmt::ChatFolderSummary> folders;

  if (!folders_arr.empty()) {
    for (const auto &item : folders_arr) {
      auto id_opt = item.get_int("id");
      if (!id_opt) continue;

      int32_t fid = static_cast<int32_t>(*id_opt);
      std::string title = extract_folder_title(item);
      if (title.empty()) title = "Folder " + std::to_string(fid);
      std::string icon = "Custom";

      if (auto icon_val = item.get_object("icon")) {
        icon = icon_val->get_string("name").value_or("Custom");
      }

      fmt::ChatFolderSummary summary;
      summary.id = fid;
      summary.title = title;
      summary.icon = icon;
      summary.color_id = static_cast<int32_t>(item.get_int("color_id").value_or(-1));

      const std::string detail_req = std::format(R"({{"chat_folder_id": {}}})", fid);
      if (auto detail_res = client_->send_request("getChatFolder", detail_req, 5.0)) {
        summary.include_groups = detail_res->get_bool("include_groups").value_or(false);
        summary.include_channels = detail_res->get_bool("include_channels").value_or(false);
        summary.include_bots = detail_res->get_bool("include_bots").value_or(false);
        summary.include_contacts = detail_res->get_bool("include_contacts").value_or(false);
        summary.include_non_contacts = detail_res->get_bool("include_non_contacts").value_or(false);
        summary.exclude_muted = detail_res->get_bool("exclude_muted").value_or(false);
        summary.exclude_read = detail_res->get_bool("exclude_read").value_or(false);
        summary.exclude_archived = detail_res->get_bool("exclude_archived").value_or(false);

        for (const auto &cid : detail_res->get_array("pinned_chat_ids")) {
          if (auto id = cid.as_int64()) summary.pinned_chat_ids.push_back(*id);
        }
        for (const auto &cid : detail_res->get_array("included_chat_ids")) {
          if (auto id = cid.as_int64()) summary.included_chat_ids.push_back(*id);
        }
        for (const auto &cid : detail_res->get_array("excluded_chat_ids")) {
          if (auto id = cid.as_int64()) summary.excluded_chat_ids.push_back(*id);
        }
      }

      folders.push_back(summary);
    }
  } else {
    for (int32_t fid = 1; fid <= 20; ++fid) {
      const std::string detail_req = std::format(R"({{"chat_folder_id": {}}})", fid);
      if (auto detail_res = client_->send_request("getChatFolder", detail_req, 1.5)) {
        fmt::ChatFolderSummary summary;
        summary.id = fid;
        summary.title = extract_folder_title(*detail_res);
        if (summary.title.empty()) summary.title = "Folder " + std::to_string(fid);
        summary.include_groups = detail_res->get_bool("include_groups").value_or(false);
        summary.include_channels = detail_res->get_bool("include_channels").value_or(false);
        summary.include_bots = detail_res->get_bool("include_bots").value_or(false);
        summary.include_contacts = detail_res->get_bool("include_contacts").value_or(false);
        summary.include_non_contacts = detail_res->get_bool("include_non_contacts").value_or(false);
        summary.exclude_muted = detail_res->get_bool("exclude_muted").value_or(false);
        summary.exclude_read = detail_res->get_bool("exclude_read").value_or(false);
        summary.exclude_archived = detail_res->get_bool("exclude_archived").value_or(false);

        for (const auto &cid : detail_res->get_array("pinned_chat_ids")) {
          if (auto id = cid.as_int64()) summary.pinned_chat_ids.push_back(*id);
        }
        for (const auto &cid : detail_res->get_array("included_chat_ids")) {
          if (auto id = cid.as_int64()) summary.included_chat_ids.push_back(*id);
        }
        for (const auto &cid : detail_res->get_array("excluded_chat_ids")) {
          if (auto id = cid.as_int64()) summary.excluded_chat_ids.push_back(*id);
        }

        folders.push_back(summary);
      }
    }
  }

  fmt::Formatter::print_folders(folders, options_.format, options_.color_mode, std::cout, verbose);
  return 0;
}

std::expected<int, std::string>
App::cmd_folder_create(const std::vector<std::string> &args) {
  if (args.empty()) {
    return std::unexpected("Usage: grm folder create <title> [options...]");
  }

  if (args[0] == "-h" || args[0] == "--help") {
    print_folder_help(options_.format);
    return 0;
  }

  std::string title = args[0];
  bool inc_groups = false;
  bool inc_channels = false;
  bool inc_bots = false;
  bool inc_contacts = false;
  bool inc_non_contacts = false;
  bool exc_muted = false;
  bool exc_read = false;
  bool exc_archived = false;

  std::vector<int64_t> included_ids;
  std::vector<int64_t> pinned_ids;

  for (size_t i = 1; i < args.size(); ++i) {
    const auto &arg = args[i];
    if (arg == "-g" || arg == "--include-groups") inc_groups = true;
    else if (arg == "-c" || arg == "--include-channels") inc_channels = true;
    else if (arg == "-b" || arg == "--include-bots") inc_bots = true;
    else if (arg == "-C" || arg == "--include-contacts") inc_contacts = true;
    else if (arg == "-N" || arg == "--include-non-contacts") inc_non_contacts = true;
    else if (arg == "--exclude-muted") exc_muted = true;
    else if (arg == "--exclude-read") exc_read = true;
    else if (arg == "--exclude-archived") exc_archived = true;
    else if ((arg == "-i" || arg == "--include-chats") && i + 1 < args.size()) {
      if (auto cid = parse_int64(args[++i])) included_ids.push_back(*cid);
    } else if ((arg == "-p" || arg == "--pinned-chats") && i + 1 < args.size()) {
      if (auto cid = parse_int64(args[++i])) pinned_ids.push_back(*cid);
    }
  }

  if (auto res = ensure_authenticated(); !res) {
    return std::unexpected(res.error());
  }

  std::string inc_json = "[";
  for (size_t i = 0; i < included_ids.size(); ++i) {
    if (i > 0) inc_json += ",";
    inc_json += std::to_string(included_ids[i]);
  }
  inc_json += "]";

  std::string pin_json = "[";
  for (size_t i = 0; i < pinned_ids.size(); ++i) {
    if (i > 0) pin_json += ",";
    pin_json += std::to_string(pinned_ids[i]);
  }
  pin_json += "]";

  const std::string payload = std::format(R"({{
    "folder": {{
      "@type": "chatFolder",
      "title": "{}",
      "icon": {{"@type": "chatFolderIcon", "name": "Custom"}},
      "color_id": -1,
      "pinned_chat_ids": {},
      "included_chat_ids": {},
      "excluded_chat_ids": [],
      "exclude_muted": {},
      "exclude_read": {},
      "exclude_archived": {},
      "include_contacts": {},
      "include_non_contacts": {},
      "include_bots": {},
      "include_groups": {},
      "include_channels": {}
    }}
  }})", escape_json_string(title), pin_json, inc_json,
        exc_muted ? "true" : "false", exc_read ? "true" : "false", exc_archived ? "true" : "false",
        inc_contacts ? "true" : "false", inc_non_contacts ? "true" : "false", inc_bots ? "true" : "false",
        inc_groups ? "true" : "false", inc_channels ? "true" : "false");

  auto res = client_->send_request("createChatFolder", payload, 10.0);
  if (!res) {
    return std::unexpected("Failed to create chat folder: " + res.error());
  }

  int32_t new_id = static_cast<int32_t>(res->get_int("id").value_or(0));

  if (options_.format == fmt::OutputFormat::Json || options_.format == fmt::OutputFormat::JsonL) {
    std::cout << std::format(R"({{"ok":true,"action":"create","id":{},"title":"{}"}})", new_id, escape_json_string(title)) << "\n";
  } else {
    std::cout << std::format("[INFO] Chat folder '{}' created successfully (ID: {})\n", title, new_id);
  }

  return 0;
}

std::expected<int, std::string>
App::cmd_folder_edit(const std::vector<std::string> &args) {
  if (args.empty()) {
    return std::unexpected("Usage: grm folder edit <folder_id> [options...]");
  }

  if (args[0] == "-h" || args[0] == "--help") {
    print_folder_help(options_.format);
    return 0;
  }

  auto fid_res = parse_int32(args[0]);
  if (!fid_res) {
    return std::unexpected(fid_res.error());
  }
  int32_t fid = *fid_res;

  if (auto res = ensure_authenticated(); !res) {
    return std::unexpected(res.error());
  }

  // Fetch existing folder configuration first
  const std::string get_req = std::format(R"({{"chat_folder_id": {}}})", fid);
  auto curr_res = client_->send_request("getChatFolder", get_req, 5.0);
  if (!curr_res) {
    return std::unexpected(std::format("Folder ID {} not found: {}", fid, curr_res.error()));
  }

  std::string title = curr_res->get_string("title").value_or("Folder " + std::to_string(fid));
  bool inc_groups = curr_res->get_bool("include_groups").value_or(false);
  bool inc_channels = curr_res->get_bool("include_channels").value_or(false);
  bool inc_bots = curr_res->get_bool("include_bots").value_or(false);
  bool inc_contacts = curr_res->get_bool("include_contacts").value_or(false);
  bool inc_non_contacts = curr_res->get_bool("include_non_contacts").value_or(false);
  bool exc_muted = curr_res->get_bool("exclude_muted").value_or(false);
  bool exc_read = curr_res->get_bool("exclude_read").value_or(false);
  bool exc_archived = curr_res->get_bool("exclude_archived").value_or(false);

  std::vector<int64_t> included_ids;
  std::vector<int64_t> pinned_ids;
  std::vector<int64_t> excluded_ids;

  for (const auto &val : curr_res->get_array("included_chat_ids")) {
    if (auto id = val.as_int64()) included_ids.push_back(*id);
  }
  for (const auto &val : curr_res->get_array("pinned_chat_ids")) {
    if (auto id = val.as_int64()) pinned_ids.push_back(*id);
  }

  for (size_t i = 1; i < args.size(); ++i) {
    const auto &arg = args[i];
    if ((arg == "-t" || arg == "--title") && i + 1 < args.size()) {
      title = args[++i];
    } else if ((arg == "-a" || arg == "--add-chat") && i + 1 < args.size()) {
      if (auto cid = parse_int64(args[++i])) included_ids.push_back(*cid);
    } else if ((arg == "-r" || arg == "--remove-chat") && i + 1 < args.size()) {
      if (auto cid = parse_int64(args[++i])) {
        std::erase(included_ids, *cid);
        std::erase(pinned_ids, *cid);
      }
    } else if ((arg == "-P" || arg == "--pin-chat") && i + 1 < args.size()) {
      if (auto cid = parse_int64(args[++i])) pinned_ids.push_back(*cid);
    } else if ((arg == "-U" || arg == "--unpin-chat") && i + 1 < args.size()) {
      if (auto cid = parse_int64(args[++i])) std::erase(pinned_ids, *cid);
    }
  }

  std::string inc_json = "[";
  for (size_t i = 0; i < included_ids.size(); ++i) {
    if (i > 0) inc_json += ",";
    inc_json += std::to_string(included_ids[i]);
  }
  inc_json += "]";

  std::string pin_json = "[";
  for (size_t i = 0; i < pinned_ids.size(); ++i) {
    if (i > 0) pin_json += ",";
    pin_json += std::to_string(pinned_ids[i]);
  }
  pin_json += "]";

  const std::string payload = std::format(R"({{
    "chat_folder_id": {},
    "folder": {{
      "@type": "chatFolder",
      "title": "{}",
      "icon": {{"@type": "chatFolderIcon", "name": "Custom"}},
      "color_id": -1,
      "pinned_chat_ids": {},
      "included_chat_ids": {},
      "excluded_chat_ids": [],
      "exclude_muted": {},
      "exclude_read": {},
      "exclude_archived": {},
      "include_contacts": {},
      "include_non_contacts": {},
      "include_bots": {},
      "include_groups": {},
      "include_channels": {}
    }}
  }})", fid, escape_json_string(title), pin_json, inc_json,
        exc_muted ? "true" : "false", exc_read ? "true" : "false", exc_archived ? "true" : "false",
        inc_contacts ? "true" : "false", inc_non_contacts ? "true" : "false", inc_bots ? "true" : "false",
        inc_groups ? "true" : "false", inc_channels ? "true" : "false");

  auto res = client_->send_request("editChatFolder", payload, 10.0);
  if (!res) {
    return std::unexpected("Failed to edit chat folder: " + res.error());
  }

  if (options_.format == fmt::OutputFormat::Json || options_.format == fmt::OutputFormat::JsonL) {
    std::cout << std::format(R"({{"ok":true,"action":"edit","id":{},"title":"{}"}})", fid, escape_json_string(title)) << "\n";
  } else {
    std::cout << std::format("[INFO] Chat folder ID {} ('{}') updated successfully\n", fid, title);
  }

  return 0;
}

std::expected<int, std::string>
App::cmd_folder_delete(const std::vector<std::string> &args) {
  if (args.empty()) {
    return std::unexpected("Usage: grm folder delete <folder_id>");
  }

  if (args[0] == "-h" || args[0] == "--help") {
    print_folder_help(options_.format);
    return 0;
  }

  auto fid_res = parse_int32(args[0]);
  if (!fid_res) {
    return std::unexpected(fid_res.error());
  }
  int32_t fid = *fid_res;

  if (auto res = ensure_authenticated(); !res) {
    return std::unexpected(res.error());
  }

  const std::string payload = std::format(R"({{"chat_folder_id": {}}})", fid);
  auto res = client_->send_request("deleteChatFolder", payload, 10.0);
  if (!res) {
    return std::unexpected(std::format("Failed to delete chat folder ID {}: {}", fid, res.error()));
  }

  if (options_.format == fmt::OutputFormat::Json || options_.format == fmt::OutputFormat::JsonL) {
    std::cout << std::format(R"({{"ok":true,"action":"delete","id":{}}})", fid) << "\n";
  } else {
    std::cout << std::format("[INFO] Chat folder ID {} deleted successfully\n", fid);
  }

  return 0;
}

} // namespace grm
