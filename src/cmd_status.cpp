#include "grm/cmd_status.hpp"
#include "grm/app.hpp"
#include "grm/formatter.hpp"
#include "grm/json_utils.hpp"
#include "grm/logger.hpp"
#include <charconv>
#include <chrono>
#include <format>
#include <iostream>

namespace grm {

CommandSpec get_status_spec() {
  return CommandSpec{
      "status",
      "Manage Telegram custom emoji statuses",
      {SubcommandSpec{
           "set",
           "--emoji <id> [--duration <time>] [-C|--chat <id>]",
           "Set custom emoji status badge",
           {OptionSpec{
                "-e",
                "--emoji",
                "<id>",
                "Custom emoji sticker/status identifier (integer string)",
                {}},
            OptionSpec{"-d",
                       "--duration",
                       "<time>",
                       "Status active duration (e.g. '1h', '30m', '1d', or "
                       "seconds; default: indefinite)",
                       {}},
            OptionSpec{
                "-C",
                "--chat",
                "<id>",
                "Target supergroup or channel ID (for boosted chat status)",
                {}},
            OptionSpec{
                "-h", "--help", "", "Show status set help message", {}}}},
       SubcommandSpec{
           "clear",
           "[-C|--chat <id>]",
           "Clear custom emoji status badge",
           {OptionSpec{
                "-C",
                "--chat",
                "<id>",
                "Target supergroup or channel ID (for boosted chat status)",
                {}},
            OptionSpec{
                "-h", "--help", "", "Show status clear help message", {}}}}},
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

std::expected<int32_t, std::string>
parse_duration_string(std::string_view str) {
  if (str.empty()) {
    return 0;
  }

  // Check simple integer seconds
  auto direct = parse_int32(str);
  if (direct && *direct >= 0) {
    return *direct;
  }

  // Parse suffix (s, m, h, d, w)
  int64_t total_secs = 0;
  size_t idx = 0;
  while (idx < str.size()) {
    size_t num_start = idx;
    while (idx < str.size() &&
           std::isdigit(static_cast<unsigned char>(str[idx]))) {
      idx++;
    }
    if (num_start == idx) {
      return std::unexpected("Invalid duration format: " + std::string(str));
    }
    std::string_view num_part = str.substr(num_start, idx - num_start);
    int32_t val = 0;
    std::from_chars(num_part.data(), num_part.data() + num_part.size(), val);

    char unit = 's';
    if (idx < str.size() &&
        !std::isdigit(static_cast<unsigned char>(str[idx]))) {
      unit =
          static_cast<char>(std::tolower(static_cast<unsigned char>(str[idx])));
      idx++;
    }

    if (unit == 's') {
      total_secs += static_cast<int64_t>(val);
    } else if (unit == 'm') {
      total_secs += static_cast<int64_t>(val) * 60;
    } else if (unit == 'h') {
      total_secs += static_cast<int64_t>(val) * 3600;
    } else if (unit == 'd') {
      total_secs += static_cast<int64_t>(val) * 86400;
    } else if (unit == 'w') {
      total_secs += static_cast<int64_t>(val) * 604800;
    } else {
      return std::unexpected("Unknown duration unit: " + std::string(1, unit));
    }
  }

  if (total_secs < 0 || total_secs > 315360000) { // Max ~10 years
    return std::unexpected("Duration out of range");
  }

  return static_cast<int32_t>(total_secs);
}

bool parse_status_set_args(const std::vector<std::string> &args,
                           StatusSetOptions &opts, std::string &err) {
  for (size_t i = 0; i < args.size(); ++i) {
    const std::string &arg = args[i];
    if (arg == "-e" || arg == "--emoji") {
      if (i + 1 >= args.size()) {
        err = "Missing argument for " + arg;
        return false;
      }
      opts.custom_emoji_id = args[++i];
    } else if (arg.starts_with("--emoji=")) {
      opts.custom_emoji_id = arg.substr(8);
    } else if (arg == "-d" || arg == "--duration") {
      if (i + 1 >= args.size()) {
        err = "Missing argument for " + arg;
        return false;
      }
      auto dur = parse_duration_string(args[++i]);
      if (!dur) {
        err = dur.error();
        return false;
      }
      opts.duration_seconds = *dur;
    } else if (arg.starts_with("--duration=")) {
      auto dur = parse_duration_string(arg.substr(11));
      if (!dur) {
        err = dur.error();
        return false;
      }
      opts.duration_seconds = *dur;
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

  if (opts.custom_emoji_id.empty()) {
    err = "Missing required option: --emoji <id>";
    return false;
  }

  return true;
}

bool parse_status_clear_args(const std::vector<std::string> &args,
                             StatusClearOptions &opts, std::string &err) {
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
    } else if (arg == "-h" || arg == "--help") {
      return false;
    } else {
      err = "Unknown option: " + arg;
      return false;
    }
  }
  return true;
}

std::string build_set_emoji_status_json(const StatusSetOptions &opts,
                                        int64_t current_time) {
  int64_t expiration_ts = 0;
  if (opts.duration_seconds > 0) {
    if (current_time == 0) {
      current_time = std::chrono::duration_cast<std::chrono::seconds>(
                         std::chrono::system_clock::now().time_since_epoch())
                         .count();
    }
    expiration_ts = current_time + opts.duration_seconds;
  }

  if (opts.chat_id != 0) {
    return std::format(
        R"({{
          "chat_id": {},
          "emoji_status": {{
            "@type": "emojiStatus",
            "type": {{"@type": "emojiStatusTypeCustomEmoji", "custom_emoji_id": "{}"}},
            "expiration_date": {}
          }}
        }})",
        opts.chat_id, escape_json_string(opts.custom_emoji_id), expiration_ts);
  }

  return std::format(
      R"({{
        "emoji_status": {{
          "@type": "emojiStatus",
          "type": {{"@type": "emojiStatusTypeCustomEmoji", "custom_emoji_id": "{}"}},
          "expiration_date": {}
        }}
      }})",
      escape_json_string(opts.custom_emoji_id), expiration_ts);
}

std::string build_clear_emoji_status_json(int64_t chat_id) {
  if (chat_id != 0) {
    return std::format(
        R"({{
          "chat_id": {},
          "emoji_status": null
        }})",
        chat_id);
  }

  return R"({
    "emoji_status": null
  })";
}

void App::print_status_help(fmt::OutputFormat format) {
  if (format == fmt::OutputFormat::Json || format == fmt::OutputFormat::JsonL) {
    std::cout << CommandRegistry::get_instance().render_command_help_json(
        "status");
  } else {
    std::cout << CommandRegistry::get_instance().render_command_help("status");
  }
}

std::expected<int, std::string>
App::cmd_status(const std::vector<std::string> &args) {
  if (args.empty() || is_help_requested(args)) {
    print_status_help(options_.format);
    return 0;
  }

  const std::string &sub = args[0];
  std::vector<std::string> sub_opts(args.begin() + 1, args.end());

  if (sub == "set") {
    return cmd_status_set(sub_opts);
  }
  if (sub == "clear" || sub == "unset" || sub == "rm") {
    return cmd_status_clear(sub_opts);
  }

  print_status_help(options_.format);
  return std::unexpected("Unknown status subcommand: " + sub);
}

std::expected<int, std::string>
App::cmd_status_set(const std::vector<std::string> &args) {
  if (is_help_requested(args)) {
    print_status_help(options_.format);
    return 0;
  }

  StatusSetOptions opts;
  std::string err;
  if (!parse_status_set_args(args, opts, err)) {
    print_status_help(options_.format);
    return std::unexpected(err.empty() ? "Invalid arguments for status set"
                                       : err);
  }

  auto auth_res = ensure_authenticated();
  if (!auth_res) {
    return std::unexpected(auth_res.error());
  }

  std::string method =
      (opts.chat_id != 0) ? "setChatEmojiStatus" : "setEmojiStatus";
  std::string req = build_set_emoji_status_json(opts);
  auto res = client_->send_request(method, req, 15.0);
  if (!res) {
    return std::unexpected("Failed to set emoji status: " + res.error());
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
    if (opts.duration_seconds > 0) {
      std::cout << std::format("✓ Custom emoji status set successfully (Emoji "
                               "ID: {}, Duration: {}s)\n",
                               opts.custom_emoji_id, opts.duration_seconds);
    } else {
      std::cout << std::format(
          "✓ Custom emoji status set successfully (Emoji ID: {}, Indefinite)\n",
          opts.custom_emoji_id);
    }
  }

  return 0;
}

std::expected<int, std::string>
App::cmd_status_clear(const std::vector<std::string> &args) {
  if (is_help_requested(args)) {
    print_status_help(options_.format);
    return 0;
  }

  StatusClearOptions opts;
  std::string err;
  if (!parse_status_clear_args(args, opts, err)) {
    print_status_help(options_.format);
    return std::unexpected(err.empty() ? "Invalid arguments for status clear"
                                       : err);
  }

  auto auth_res = ensure_authenticated();
  if (!auth_res) {
    return std::unexpected(auth_res.error());
  }

  std::string method =
      (opts.chat_id != 0) ? "setChatEmojiStatus" : "setEmojiStatus";
  std::string req = build_clear_emoji_status_json(opts.chat_id);
  auto res = client_->send_request(method, req, 15.0);
  if (!res) {
    return std::unexpected("Failed to clear emoji status: " + res.error());
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
    std::cout << "✓ Custom emoji status cleared successfully\n";
  }

  return 0;
}

} // namespace grm
