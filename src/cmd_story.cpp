#include "grm/cmd_story.hpp"
#include "grm/app.hpp"
#include "grm/formatter.hpp"
#include "grm/json_utils.hpp"
#include "grm/logger.hpp"
#include <charconv>
#include <filesystem>
#include <format>
#include <iostream>
#include <json-c/json.h>
#include <unordered_set>

namespace grm {

CommandSpec get_story_spec() {
  return CommandSpec{
      "story",
      "Manage and publish Telegram Stories",
      {SubcommandSpec{
           "post",
           "[--photo <path>|--video <path>] [--caption <caption>] [--privacy "
           "everyone|contacts|close_friends] [--period <time>] [--pinned] "
           "[--protect] [--chat <id>]",
           "Publish a new photo or video story",
           {OptionSpec{"-p",
                       "--photo",
                       "<path>",
                       "Path to photo image file to publish as story",
                       {}},
            OptionSpec{"-v",
                       "--video",
                       "<path>",
                       "Path to video file to publish as story",
                       {}},
            OptionSpec{"-c",
                       "--caption",
                       "<caption>",
                       "Story caption (supports Markdown formatting)",
                       {}},
            OptionSpec{"",
                       "--privacy",
                       "<setting>",
                       "Privacy setting (everyone, contacts, close_friends; "
                       "default: everyone)",
                       {"everyone", "contacts", "close_friends"}},
            OptionSpec{
                "",
                "--period",
                "<time>",
                "Active story duration (6h, 12h, 24h, 48h; default: 24h)",
                {"6h", "12h", "24h", "48h"}},
            OptionSpec{"",
                       "--pinned",
                       "",
                       "Post story to chat page / profile after expiration",
                       {}},
            OptionSpec{"",
                       "--protect",
                       "",
                       "Protect story content from saving and forwarding",
                       {}},
            OptionSpec{"-C",
                       "--chat",
                       "<id>",
                       "Target chat or channel ID (default: personal account)",
                       {}},
            OptionSpec{
                "-h", "--help", "", "Show story post help message", {}}}},
       SubcommandSpec{
           "ls",
           "[-C|--chat <id>] [-n|--limit <N>] [-p|--pinned] [-a|--archived] "
           "[-A|--all]",
           "List active, pinned profile, and archived stories",
           {OptionSpec{"-C",
                       "--chat",
                       "<id>",
                       "Target chat or channel ID (default: personal account)",
                       {}},
            OptionSpec{"-n",
                       "--limit",
                       "<N>",
                       "Maximum number of stories to display (default: 20)",
                       {}},
            OptionSpec{"-p",
                       "--pinned",
                       "",
                       "List stories posted to profile / chat page",
                       {}},
            OptionSpec{"-a", "--archived", "", "List archived stories", {}},
            OptionSpec{"-A",
                       "--all",
                       "",
                       "List active, pinned, and archived stories",
                       {}},
            OptionSpec{
                "-h", "--help", "", "Show story list help message", {}}}},
       SubcommandSpec{
           "edit",
           "--story-id <id> [--photo <path>|--video <path>] [--caption "
           "<caption>] [-C|--chat <id>]",
           "Edit content or caption of a posted story",
           {OptionSpec{
                "-s", "--story-id", "<id>", "Story identifier to edit", {}},
            OptionSpec{"-p",
                       "--photo",
                       "<path>",
                       "New photo image file for story",
                       {}},
            OptionSpec{
                "-v", "--video", "<path>", "New video file for story", {}},
            OptionSpec{"-c",
                       "--caption",
                       "<caption>",
                       "New story caption (supports Markdown formatting)",
                       {}},
            OptionSpec{"-C",
                       "--chat",
                       "<id>",
                       "Target chat or channel ID (default: personal account)",
                       {}},
            OptionSpec{
                "-h", "--help", "", "Show story edit help message", {}}}},
       SubcommandSpec{
           "delete",
           "--story-id <id> [-C|--chat <id>]",
           "Delete a posted story",
           {OptionSpec{
                "-s", "--story-id", "<id>", "Story identifier to delete", {}},
            OptionSpec{"-C",
                       "--chat",
                       "<id>",
                       "Target chat or channel ID (default: personal account)",
                       {}},
            OptionSpec{
                "-h", "--help", "", "Show story delete help message", {}}}}},
      {}};
}

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

std::expected<int32_t, std::string> parse_period_string(std::string_view str) {
  if (str.empty()) {
    return 86400;
  }
  if (str == "6h" || str == "6" || str == "21600") {
    return 21600;
  }
  if (str == "12h" || str == "12" || str == "43200") {
    return 43200;
  }
  if (str == "24h" || str == "1d" || str == "24" || str == "86400") {
    return 86400;
  }
  if (str == "48h" || str == "2d" || str == "48" || str == "172800") {
    return 172800;
  }
  auto parsed = parse_int32(str);
  if (parsed && (*parsed == 21600 || *parsed == 43200 || *parsed == 86400 ||
                 *parsed == 172800)) {
    return *parsed;
  }
  return std::unexpected(
      "Invalid story period: " + std::string(str) +
      ". Supported: 6h (21600s), 12h (43200s), 24h (86400s), 48h (172800s)");
}

bool parse_story_post_args(const std::vector<std::string> &args,
                           StoryPostOptions &opts, std::string &err) {
  for (size_t i = 0; i < args.size(); ++i) {
    const std::string &arg = args[i];
    if (arg == "-p" || arg == "--photo") {
      if (i + 1 >= args.size()) {
        err = "Missing argument for " + arg;
        return false;
      }
      opts.photo_path = args[++i];
    } else if (arg.starts_with("--photo=")) {
      opts.photo_path = arg.substr(8);
    } else if (arg == "-v" || arg == "--video") {
      if (i + 1 >= args.size()) {
        err = "Missing argument for " + arg;
        return false;
      }
      opts.video_path = args[++i];
    } else if (arg.starts_with("--video=")) {
      opts.video_path = arg.substr(8);
    } else if (arg == "-c" || arg == "--caption") {
      if (i + 1 >= args.size()) {
        err = "Missing argument for " + arg;
        return false;
      }
      opts.caption = args[++i];
    } else if (arg.starts_with("--caption=")) {
      opts.caption = arg.substr(10);
    } else if (arg == "--privacy") {
      if (i + 1 >= args.size()) {
        err = "Missing argument for " + arg;
        return false;
      }
      opts.privacy = args[++i];
    } else if (arg.starts_with("--privacy=")) {
      opts.privacy = arg.substr(10);
    } else if (arg == "--period") {
      if (i + 1 >= args.size()) {
        err = "Missing argument for " + arg;
        return false;
      }
      auto p = parse_period_string(args[++i]);
      if (!p) {
        err = p.error();
        return false;
      }
      opts.active_period = *p;
    } else if (arg.starts_with("--period=")) {
      auto p = parse_period_string(arg.substr(9));
      if (!p) {
        err = p.error();
        return false;
      }
      opts.active_period = *p;
    } else if (arg == "--pinned") {
      opts.is_pinned = true;
    } else if (arg == "--protect") {
      opts.protect_content = true;
    } else if (arg == "-C" || arg == "--chat") {
      if (i + 1 >= args.size()) {
        err = "Missing argument for " + arg;
        return false;
      }
      auto chat_res = parse_int64(args[++i]);
      if (!chat_res) {
        err = chat_res.error();
        return false;
      }
      opts.chat_id = *chat_res;
    } else if (arg.starts_with("--chat=")) {
      auto chat_res = parse_int64(arg.substr(7));
      if (!chat_res) {
        err = chat_res.error();
        return false;
      }
      opts.chat_id = *chat_res;
    } else if (arg == "-h" || arg == "--help") {
      return false;
    } else {
      err = "Unknown option: " + arg;
      return false;
    }
  }

  if (opts.photo_path.empty() && opts.video_path.empty()) {
    err = "Either --photo <path> or --video <path> must be specified";
    return false;
  }
  if (!opts.photo_path.empty() && !opts.video_path.empty()) {
    err = "Cannot specify both --photo and --video simultaneously";
    return false;
  }
  if (opts.privacy != "everyone" && opts.privacy != "contacts" &&
      opts.privacy != "close_friends") {
    err = "Invalid privacy setting: '" + opts.privacy +
          "'. Expected: everyone, contacts, close_friends";
    return false;
  }

  return true;
}

bool parse_story_ls_args(const std::vector<std::string> &args,
                         StoryListOptions &opts, std::string &err) {
  for (size_t i = 0; i < args.size(); ++i) {
    const std::string &arg = args[i];
    if (arg == "-C" || arg == "--chat") {
      if (i + 1 >= args.size()) {
        err = "Missing argument for " + arg;
        return false;
      }
      auto chat_res = parse_int64(args[++i]);
      if (!chat_res) {
        err = chat_res.error();
        return false;
      }
      opts.chat_id = *chat_res;
    } else if (arg.starts_with("--chat=")) {
      auto chat_res = parse_int64(arg.substr(7));
      if (!chat_res) {
        err = chat_res.error();
        return false;
      }
      opts.chat_id = *chat_res;
    } else if (arg == "-n" || arg == "--limit") {
      if (i + 1 >= args.size()) {
        err = "Missing argument for " + arg;
        return false;
      }
      auto lim = parse_int32(args[++i]);
      if (!lim || *lim <= 0) {
        err = "Invalid limit: " + args[i];
        return false;
      }
      opts.limit = *lim;
    } else if (arg.starts_with("--limit=")) {
      auto lim = parse_int32(arg.substr(8));
      if (!lim || *lim <= 0) {
        err = "Invalid limit: " + arg.substr(8);
        return false;
      }
      opts.limit = *lim;
    } else if (arg == "-p" || arg == "--pinned" || arg == "--posts") {
      opts.pinned = true;
    } else if (arg == "-a" || arg == "--archived") {
      opts.archived = true;
    } else if (arg == "-A" || arg == "--all") {
      opts.all = true;
    } else if (arg == "-h" || arg == "--help") {
      return false;
    } else {
      err = "Unknown option: " + arg;
      return false;
    }
  }
  return true;
}

bool parse_story_edit_args(const std::vector<std::string> &args,
                           StoryEditOptions &opts, std::string &err) {
  for (size_t i = 0; i < args.size(); ++i) {
    const std::string &arg = args[i];
    if (arg == "-s" || arg == "--story-id") {
      if (i + 1 >= args.size()) {
        err = "Missing argument for " + arg;
        return false;
      }
      auto sid = parse_int32(args[++i]);
      if (!sid || *sid <= 0) {
        err = "Invalid story ID: " + args[i];
        return false;
      }
      opts.story_id = *sid;
    } else if (arg.starts_with("--story-id=")) {
      auto sid = parse_int32(arg.substr(11));
      if (!sid || *sid <= 0) {
        err = "Invalid story ID: " + arg.substr(11);
        return false;
      }
      opts.story_id = *sid;
    } else if (arg == "-p" || arg == "--photo") {
      if (i + 1 >= args.size()) {
        err = "Missing argument for " + arg;
        return false;
      }
      opts.photo_path = args[++i];
    } else if (arg.starts_with("--photo=")) {
      opts.photo_path = arg.substr(8);
    } else if (arg == "-v" || arg == "--video") {
      if (i + 1 >= args.size()) {
        err = "Missing argument for " + arg;
        return false;
      }
      opts.video_path = args[++i];
    } else if (arg.starts_with("--video=")) {
      opts.video_path = arg.substr(8);
    } else if (arg == "-c" || arg == "--caption") {
      if (i + 1 >= args.size()) {
        err = "Missing argument for " + arg;
        return false;
      }
      opts.caption = args[++i];
      opts.has_caption = true;
    } else if (arg.starts_with("--caption=")) {
      opts.caption = arg.substr(10);
      opts.has_caption = true;
    } else if (arg == "-C" || arg == "--chat") {
      if (i + 1 >= args.size()) {
        err = "Missing argument for " + arg;
        return false;
      }
      auto chat_res = parse_int64(args[++i]);
      if (!chat_res) {
        err = chat_res.error();
        return false;
      }
      opts.chat_id = *chat_res;
    } else if (arg.starts_with("--chat=")) {
      auto chat_res = parse_int64(arg.substr(7));
      if (!chat_res) {
        err = chat_res.error();
        return false;
      }
      opts.chat_id = *chat_res;
    } else if (arg == "-h" || arg == "--help") {
      return false;
    } else {
      err = "Unknown option: " + arg;
      return false;
    }
  }

  if (opts.story_id <= 0) {
    err = "Missing required option: --story-id <id>";
    return false;
  }
  if (!opts.photo_path.empty() && !opts.video_path.empty()) {
    err = "Cannot specify both --photo and --video";
    return false;
  }
  if (opts.photo_path.empty() && opts.video_path.empty() && !opts.has_caption) {
    err = "Must specify at least one of --photo, --video, or --caption to edit";
    return false;
  }

  return true;
}

bool parse_story_delete_args(const std::vector<std::string> &args,
                             StoryDeleteOptions &opts, std::string &err) {
  for (size_t i = 0; i < args.size(); ++i) {
    const std::string &arg = args[i];
    if (arg == "-s" || arg == "--story-id") {
      if (i + 1 >= args.size()) {
        err = "Missing argument for " + arg;
        return false;
      }
      auto sid = parse_int32(args[++i]);
      if (!sid || *sid <= 0) {
        err = "Invalid story ID: " + args[i];
        return false;
      }
      opts.story_id = *sid;
    } else if (arg.starts_with("--story-id=")) {
      auto sid = parse_int32(arg.substr(11));
      if (!sid || *sid <= 0) {
        err = "Invalid story ID: " + arg.substr(11);
        return false;
      }
      opts.story_id = *sid;
    } else if (arg == "-C" || arg == "--chat") {
      if (i + 1 >= args.size()) {
        err = "Missing argument for " + arg;
        return false;
      }
      auto chat_res = parse_int64(args[++i]);
      if (!chat_res) {
        err = chat_res.error();
        return false;
      }
      opts.chat_id = *chat_res;
    } else if (arg.starts_with("--chat=")) {
      auto chat_res = parse_int64(arg.substr(7));
      if (!chat_res) {
        err = chat_res.error();
        return false;
      }
      opts.chat_id = *chat_res;
    } else if (arg == "-h" || arg == "--help") {
      return false;
    } else {
      err = "Unknown option: " + arg;
      return false;
    }
  }

  if (opts.story_id <= 0) {
    err = "Missing required option: --story-id <id>";
    return false;
  }
  return true;
}

std::string build_post_story_json(const StoryPostOptions &opts,
                                  int64_t resolved_chat_id,
                                  const std::string &formatted_caption_json) {
  std::string privacy_type = "storyPrivacySettingsEveryone";
  if (opts.privacy == "contacts") {
    privacy_type = "storyPrivacySettingsContacts";
  } else if (opts.privacy == "close_friends") {
    privacy_type = "storyPrivacySettingsCloseFriends";
  }

  std::string content_json;
  if (!opts.photo_path.empty()) {
    std::filesystem::path p(opts.photo_path);
    std::string abs_path = std::filesystem::absolute(p).string();
    content_json = std::format(
        R"({{"@type":"inputStoryContentPhoto","photo":{{"@type":"inputFileLocal","path":"{}"}},"added_sticker_file_ids":[],"is_animation":false}})",
        escape_json_string(abs_path));
  } else {
    std::filesystem::path p(opts.video_path);
    std::string abs_path = std::filesystem::absolute(p).string();
    content_json = std::format(
        R"({{"@type":"inputStoryContentVideo","video":{{"@type":"inputFileLocal","path":"{}"}},"added_sticker_file_ids":[],"is_animation":false}})",
        escape_json_string(abs_path));
  }

  std::string caption_part =
      formatted_caption_json.empty()
          ? R"({"@type":"formattedText","text":"","entities":[]})"
          : formatted_caption_json;

  return std::format(
      R"({{
        "chat_id": {},
        "content": {},
        "areas": null,
        "caption": {},
        "privacy_settings": {{"@type":"{}"}},
        "album_ids": [],
        "active_period": {},
        "from_story_full_id": null,
        "is_posted_to_chat_page": {},
        "protect_content": {}
      }})",
      resolved_chat_id, content_json, caption_part, privacy_type,
      opts.active_period, opts.is_pinned ? "true" : "false",
      opts.protect_content ? "true" : "false");
}

std::string build_edit_story_json(int64_t chat_id, int32_t story_id,
                                  const StoryEditOptions &opts,
                                  const std::string &formatted_caption_json) {
  std::string content_json = "null";
  if (!opts.photo_path.empty()) {
    std::filesystem::path p(opts.photo_path);
    std::string abs_path = std::filesystem::absolute(p).string();
    content_json = std::format(
        R"({{"@type":"inputStoryContentPhoto","photo":{{"@type":"inputFileLocal","path":"{}"}},"added_sticker_file_ids":[],"is_animation":false}})",
        escape_json_string(abs_path));
  } else if (!opts.video_path.empty()) {
    std::filesystem::path p(opts.video_path);
    std::string abs_path = std::filesystem::absolute(p).string();
    content_json = std::format(
        R"({{"@type":"inputStoryContentVideo","video":{{"@type":"inputFileLocal","path":"{}"}},"added_sticker_file_ids":[],"is_animation":false}})",
        escape_json_string(abs_path));
  }

  std::string caption_json =
      opts.has_caption
          ? (formatted_caption_json.empty()
                 ? R"({"@type":"formattedText","text":"","entities":[]})"
                 : formatted_caption_json)
          : "null";

  return std::format(
      R"({{
        "story_poster_chat_id": {},
        "story_id": {},
        "content": {},
        "areas": null,
        "caption": {}
      }})",
      chat_id, story_id, content_json, caption_json);
}

std::string build_get_chat_active_stories_json(int64_t chat_id) {
  return std::format(
      R"({{
        "chat_id": {}
      }})",
      chat_id);
}

std::string build_delete_story_json(int64_t chat_id, int32_t story_id) {
  return std::format(
      R"({{
        "story_poster_chat_id": {},
        "story_id": {}
      }})",
      chat_id, story_id);
}

void App::print_story_help(fmt::OutputFormat format) {
  if (format == fmt::OutputFormat::Json || format == fmt::OutputFormat::JsonL) {
    std::cout << CommandRegistry::get_instance().render_command_help_json(
        "story");
  } else {
    std::cout << CommandRegistry::get_instance().render_command_help("story");
  }
}

std::expected<int, std::string>
App::cmd_story(const std::vector<std::string> &args) {
  if (args.empty() || is_help_requested(args)) {
    print_story_help(options_.format);
    return 0;
  }

  const std::string &sub = args[0];
  std::vector<std::string> sub_opts(args.begin() + 1, args.end());

  if (sub == "post") {
    return cmd_story_post(sub_opts);
  }
  if (sub == "edit" || sub == "update") {
    return cmd_story_edit(sub_opts);
  }
  if (sub == "ls" || sub == "list") {
    return cmd_story_ls(sub_opts);
  }
  if (sub == "delete" || sub == "rm") {
    return cmd_story_delete(sub_opts);
  }

  print_story_help(options_.format);
  return std::unexpected("Unknown story subcommand: " + sub);
}

std::expected<int, std::string>
App::cmd_story_post(const std::vector<std::string> &args) {
  if (is_help_requested(args)) {
    print_story_help(options_.format);
    return 0;
  }

  StoryPostOptions opts;
  std::string err;
  if (!parse_story_post_args(args, opts, err)) {
    print_story_help(options_.format);
    return std::unexpected(err.empty() ? "Invalid arguments for story post"
                                       : err);
  }

  // Validate local media file existence
  std::string media_path =
      !opts.photo_path.empty() ? opts.photo_path : opts.video_path;
  if (!std::filesystem::exists(media_path)) {
    return std::unexpected("Media file not found: " + media_path);
  }

  auto auth_res = ensure_authenticated();
  if (!auth_res) {
    return std::unexpected(auth_res.error());
  }

  int64_t target_chat_id = opts.chat_id;
  if (target_chat_id == 0) {
    // Resolve personal chat ID via getMe
    auto me_res = client_->send_request("getMe", "{}", 10.0);
    if (!me_res) {
      return std::unexpected("Failed to resolve current user profile: " +
                             me_res.error());
    }
    target_chat_id = me_res->get_int("id").value_or(0);
    if (target_chat_id == 0) {
      return std::unexpected("Failed to extract user ID from getMe response");
    }
  }

  std::string formatted_caption_json;
  if (!opts.caption.empty()) {
    auto parsed_caption = parse_formatted_text(opts.caption, "markdown");
    if (!parsed_caption) {
      return std::unexpected("Failed to parse caption entities: " +
                             parsed_caption.error());
    }
    formatted_caption_json = parsed_caption->to_string();
  }

  std::string post_req =
      build_post_story_json(opts, target_chat_id, formatted_caption_json);
  grm::log::debug("Dispatching postStory request...");
  auto post_res = client_->send_request("postStory", post_req, 30.0);
  if (!post_res) {
    return std::unexpected("Failed to post story: " + post_res.error());
  }

  std::string res_type = post_res->get_string("@type").value_or("");
  if (res_type == "error") {
    int64_t code = post_res->get_int("code").value_or(0);
    std::string msg = post_res->get_string("message").value_or("Unknown error");
    return std::unexpected(std::format("TDLib Error [{}]: {}", code, msg));
  }

  int64_t story_id = post_res->get_int("id").value_or(0);

  if (options_.format == fmt::OutputFormat::Json ||
      options_.format == fmt::OutputFormat::JsonL) {
    std::cout << post_res->to_string() << "\n";
  } else {
    std::cout << std::format("✓ Story published successfully (Story ID: {}, "
                             "Chat: {}, Privacy: {}, Active Period: {}s)\n",
                             story_id, target_chat_id, opts.privacy,
                             opts.active_period);
  }

  return 0;
}

std::expected<int, std::string>
App::cmd_story_edit(const std::vector<std::string> &args) {
  if (is_help_requested(args)) {
    print_story_help(options_.format);
    return 0;
  }

  StoryEditOptions opts;
  std::string err;
  if (!parse_story_edit_args(args, opts, err)) {
    print_story_help(options_.format);
    return std::unexpected(err.empty() ? "Invalid arguments for story edit"
                                       : err);
  }

  if (!opts.photo_path.empty() && !std::filesystem::exists(opts.photo_path)) {
    return std::unexpected("Photo file not found: " + opts.photo_path);
  }
  if (!opts.video_path.empty() && !std::filesystem::exists(opts.video_path)) {
    return std::unexpected("Video file not found: " + opts.video_path);
  }

  auto auth_res = ensure_authenticated();
  if (!auth_res) {
    return std::unexpected(auth_res.error());
  }

  int64_t target_chat_id = opts.chat_id;
  if (target_chat_id == 0) {
    auto me_res = client_->send_request("getMe", "{}", 10.0);
    if (!me_res) {
      return std::unexpected("Failed to resolve current user profile: " +
                             me_res.error());
    }
    target_chat_id = me_res->get_int("id").value_or(0);
    if (target_chat_id == 0) {
      return std::unexpected("Failed to extract user ID from getMe response");
    }
  }

  std::string formatted_caption_json;
  if (opts.has_caption && !opts.caption.empty()) {
    auto parsed_caption = parse_formatted_text(opts.caption, "markdown");
    if (!parsed_caption) {
      return std::unexpected("Failed to parse caption entities: " +
                             parsed_caption.error());
    }
    formatted_caption_json = parsed_caption->to_string();
  }

  // Ensure story is loaded in TDLib before editing
  std::string get_story_req =
      std::format(R"({{"story_poster_chat_id": {}, "story_id": {}}})",
                  target_chat_id, opts.story_id);
  [[maybe_unused]] auto prefetch_res =
      client_->send_request("getStory", get_story_req, 10.0);

  std::string edit_req = build_edit_story_json(target_chat_id, opts.story_id,
                                               opts, formatted_caption_json);
  grm::log::debug(std::format("Dispatching editStory request: {}", edit_req));
  auto edit_res = client_->send_request("editStory", edit_req, 30.0);
  if (!edit_res) {
    return std::unexpected("Failed to edit story: " + edit_res.error());
  }

  std::string res_type = edit_res->get_string("@type").value_or("");
  if (res_type == "error") {
    int64_t code = edit_res->get_int("code").value_or(0);
    std::string msg = edit_res->get_string("message").value_or("Unknown error");
    return std::unexpected(std::format("TDLib Error [{}]: {}", code, msg));
  }

  if (options_.format == fmt::OutputFormat::Json ||
      options_.format == fmt::OutputFormat::JsonL) {
    std::cout << edit_res->to_string() << "\n";
  } else {
    std::cout << std::format("✓ Story {} edited successfully (Chat: {})\n",
                             opts.story_id, target_chat_id);
  }

  return 0;
}

std::expected<int, std::string>
App::cmd_story_ls(const std::vector<std::string> &args) {
  if (is_help_requested(args)) {
    print_story_help(options_.format);
    return 0;
  }

  StoryListOptions opts;
  std::string err;
  if (!parse_story_ls_args(args, opts, err)) {
    print_story_help(options_.format);
    return std::unexpected(err.empty() ? "Invalid arguments for story ls"
                                       : err);
  }

  auto auth_res = ensure_authenticated();
  if (!auth_res) {
    return std::unexpected(auth_res.error());
  }

  int64_t target_chat_id = opts.chat_id;
  if (target_chat_id == 0) {
    auto me_res = client_->send_request("getMe", "{}", 10.0);
    if (!me_res) {
      return std::unexpected("Failed to resolve current user profile: " +
                             me_res.error());
    }
    target_chat_id = me_res->get_int("id").value_or(0);
  }
  struct StoryItem {
    int64_t id{0};
    int64_t date{0};
    bool is_pinned{false};
    std::string content_type{"Photo"};
    std::string caption;
  };
  std::vector<StoryItem> collected_stories;
  std::unordered_set<int64_t> seen_ids;

  // 1. Fetch active stories
  if (!opts.pinned && !opts.archived) {
    std::string req = build_get_chat_active_stories_json(target_chat_id);
    auto res = client_->send_request("getChatActiveStories", req, 15.0);
    if (res && res->get_string("@type").value_or("") != "error") {
      auto stories_array = res->get_array("stories");
      for (const auto &story_info : stories_array) {
        int64_t sid = story_info.get_int("story_id")
                          .value_or(story_info.get_int("id").value_or(0));
        if (sid > 0 && seen_ids.insert(sid).second) {
          int64_t date_ts = story_info.get_int("date").value_or(0);
          std::string full_story_req =
              std::format(R"({{"story_poster_chat_id": {}, "story_id": {}}})",
                          target_chat_id, sid);
          auto story_detail =
              client_->send_request("getStory", full_story_req, 5.0);
          const JsonValue &detail_ref =
              (story_detail &&
               story_detail->get_string("@type").value_or("") != "error")
                  ? *story_detail
                  : story_info;

          bool is_pinned =
              detail_ref.get_bool("is_posted_to_chat_page")
                  .value_or(detail_ref.get_bool("is_pinned").value_or(false));
          std::string content_type = "Photo";
          if (auto content_obj = detail_ref.get_object("content")) {
            std::string ctype = content_obj->get_string("@type").value_or("");
            if (ctype == "storyContentPhoto")
              content_type = "Photo";
            else if (ctype == "storyContentVideo")
              content_type = "Video";
          }
          std::string caption_text;
          if (auto caption_obj = detail_ref.get_object("caption")) {
            caption_text = caption_obj->get_string("text").value_or("");
          }
          collected_stories.push_back(StoryItem{
              .id = sid,
              .date = date_ts,
              .is_pinned = is_pinned,
              .content_type = content_type,
              .caption = caption_text,
          });
        }
      }
    }
  }

  // 2. Fetch posted to chat page (profile stories/posts)
  if (opts.pinned || opts.all || collected_stories.empty()) {
    std::string req =
        std::format(R"({{"chat_id": {}, "from_story_id": 0, "limit": {}}})",
                    target_chat_id, opts.limit);
    auto res =
        client_->send_request("getChatPostedToChatPageStories", req, 15.0);
    if (res && res->get_string("@type").value_or("") != "error") {
      auto stories_array = res->get_array("stories");
      for (const auto &story_obj : stories_array) {
        int64_t sid = story_obj.get_int("id").value_or(0);
        if (sid > 0 && seen_ids.insert(sid).second) {
          int64_t date_ts = story_obj.get_int("date").value_or(0);
          bool is_pinned =
              story_obj.get_bool("is_posted_to_chat_page")
                  .value_or(story_obj.get_bool("is_pinned").value_or(false));
          std::string content_type = "Photo";
          if (auto content_obj = story_obj.get_object("content")) {
            std::string ctype = content_obj->get_string("@type").value_or("");
            if (ctype == "storyContentPhoto")
              content_type = "Photo";
            else if (ctype == "storyContentVideo")
              content_type = "Video";
          }
          std::string caption_text;
          if (auto caption_obj = story_obj.get_object("caption")) {
            caption_text = caption_obj->get_string("text").value_or("");
          }
          collected_stories.push_back(StoryItem{
              .id = sid,
              .date = date_ts,
              .is_pinned = is_pinned,
              .content_type = content_type,
              .caption = caption_text,
          });
        }
      }
    }
  }

  // 3. Fetch archived stories if requested
  if (opts.archived || opts.all) {
    std::string req =
        std::format(R"({{"chat_id": {}, "from_story_id": 0, "limit": {}}})",
                    target_chat_id, opts.limit);
    auto res = client_->send_request("getChatArchivedStories", req, 15.0);
    if (res && res->get_string("@type").value_or("") != "error") {
      auto stories_array = res->get_array("stories");
      for (const auto &story_obj : stories_array) {
        int64_t sid = story_obj.get_int("id").value_or(0);
        if (sid > 0 && seen_ids.insert(sid).second) {
          int64_t date_ts = story_obj.get_int("date").value_or(0);
          bool is_pinned =
              story_obj.get_bool("is_posted_to_chat_page")
                  .value_or(story_obj.get_bool("is_pinned").value_or(false));
          std::string content_type = "Photo";
          if (auto content_obj = story_obj.get_object("content")) {
            std::string ctype = content_obj->get_string("@type").value_or("");
            if (ctype == "storyContentPhoto")
              content_type = "Photo";
            else if (ctype == "storyContentVideo")
              content_type = "Video";
          }
          std::string caption_text;
          if (auto caption_obj = story_obj.get_object("caption")) {
            caption_text = caption_obj->get_string("text").value_or("");
          }
          collected_stories.push_back(StoryItem{
              .id = sid,
              .date = date_ts,
              .is_pinned = is_pinned,
              .content_type = content_type,
              .caption = caption_text,
          });
        }
      }
    }
  }

  if (collected_stories.empty()) {
    std::cout << "No stories found for chat ID " << target_chat_id << ".\n";
    return 0;
  }

  if (options_.format == fmt::OutputFormat::Json ||
      options_.format == fmt::OutputFormat::JsonL) {
    json_object *arr = json_object_new_array();
    for (const auto &item : collected_stories) {
      json_object *obj = json_object_new_object();
      json_object_object_add(obj, "story_id", json_object_new_int64(item.id));
      json_object_object_add(obj, "date", json_object_new_int64(item.date));
      json_object_object_add(obj, "is_pinned",
                             json_object_new_boolean(item.is_pinned));
      json_object_object_add(obj, "content_type",
                             json_object_new_string(item.content_type.c_str()));
      json_object_object_add(obj, "caption",
                             json_object_new_string(item.caption.c_str()));
      json_object_array_add(arr, obj);
    }
    int flags = options_.pretty
                    ? (JSON_C_TO_STRING_PRETTY | JSON_C_TO_STRING_SPACED)
                    : JSON_C_TO_STRING_PLAIN;
    std::cout << json_object_to_json_string_ext(arr, flags) << "\n";
    json_object_put(arr);
    return 0;
  }

  size_t count =
      std::min(collected_stories.size(), static_cast<size_t>(opts.limit));
  std::cout << std::format("Found {} story/stories for chat ID {}:\n\n", count,
                           target_chat_id);
  std::cout << std::format("{:<12} {:<20} {:<8} {:<14} {}\n", "STORY ID",
                           "DATE", "PINNED", "CONTENT", "CAPTION");
  std::cout << std::string(80, '-') << "\n";

  for (size_t i = 0; i < count; ++i) {
    const auto &item = collected_stories[i];
    std::string caption_text = item.caption;
    if (caption_text.size() > 35) {
      caption_text = caption_text.substr(0, 32) + "...";
    }

    std::string date_str = std::to_string(item.date);
    if (item.date > 0) {
      std::time_t t = static_cast<std::time_t>(item.date);
      std::tm tm_buf{};
      if (localtime_r(&t, &tm_buf)) {
        char buf[32];
        if (std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M", &tm_buf) > 0) {
          date_str = buf;
        }
      }
    }

    std::cout << std::format("{:<12} {:<20} {:<8} {:<14} {}\n", item.id,
                             date_str, item.is_pinned ? "Yes" : "No",
                             item.content_type, caption_text);
  }

  return 0;
}

std::expected<int, std::string>
App::cmd_story_delete(const std::vector<std::string> &args) {
  if (is_help_requested(args)) {
    print_story_help(options_.format);
    return 0;
  }

  StoryDeleteOptions opts;
  std::string err;
  if (!parse_story_delete_args(args, opts, err)) {
    print_story_help(options_.format);
    return std::unexpected(err.empty() ? "Invalid arguments for story delete"
                                       : err);
  }

  auto auth_res = ensure_authenticated();
  if (!auth_res) {
    return std::unexpected(auth_res.error());
  }

  int64_t target_chat_id = opts.chat_id;
  if (target_chat_id == 0) {
    auto me_res = client_->send_request("getMe", "{}", 10.0);
    if (!me_res) {
      return std::unexpected("Failed to resolve current user profile: " +
                             me_res.error());
    }
    target_chat_id = me_res->get_int("id").value_or(0);
  }

  std::string req = build_delete_story_json(target_chat_id, opts.story_id);
  auto res = client_->send_request("deleteStory", req, 15.0);
  if (!res) {
    return std::unexpected("Failed to delete story: " + res.error());
  }

  if (res->get_string("@type").value_or("") == "error") {
    int64_t code = res->get_int("code").value_or(0);
    std::string msg = res->get_string("message").value_or("Unknown error");
    return std::unexpected(std::format("TDLib Error [{}]: {}", code, msg));
  }

  if (options_.format == fmt::OutputFormat::Json ||
      options_.format == fmt::OutputFormat::JsonL) {
    std::cout << res->to_string() << "\n";
  } else {
    std::cout << std::format("✓ Story {} deleted successfully (Chat: {})\n",
                             opts.story_id, target_chat_id);
  }

  return 0;
}

} // namespace grm
