#include "grm/list_options.hpp"
#include "grm/app.hpp"
#include <charconv>
#include <format>

namespace grm {

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

std::expected<ListOptions, std::string>
ListOptions::parse(const std::vector<std::string> &args,
                   std::vector<std::string> &positional_args) {
  ListOptions opts;

  for (size_t i = 0; i < args.size(); ++i) {
    std::string_view arg(args[i]);
    if ((arg == "-n" || arg == "--limit") && i + 1 < args.size()) {
      if (auto lim = parse_int32(args[++i])) {
        opts.limit = *lim;
      }
    } else if (arg.starts_with("--limit=")) {
      if (auto lim = parse_int32(arg.substr(8))) {
        opts.limit = *lim;
      }
    } else if ((arg == "-t" || arg == "--topic") && i + 1 < args.size()) {
      if (auto tid = parse_int64(args[++i])) {
        opts.topic_id = *tid;
      }
    } else if (arg.starts_with("--topic=")) {
      if (auto tid = parse_int64(arg.substr(8))) {
        opts.topic_id = *tid;
      }
    } else if ((arg == "-S" || arg == "--since") && i + 1 < args.size()) {
      if (auto ts = App::parse_since_timestamp(args[++i])) {
        opts.since_timestamp = *ts;
      } else {
        return std::unexpected(ts.error());
      }
    } else if (arg.starts_with("--since=")) {
      if (auto ts = App::parse_since_timestamp(arg.substr(8))) {
        opts.since_timestamp = *ts;
      } else {
        return std::unexpected(ts.error());
      }
    } else if ((arg == "--filter" || arg == "--sender") &&
               i + 1 < args.size()) {
      opts.filter_patterns.push_back(args[++i]);
    } else if (arg.starts_with("--filter=")) {
      opts.filter_patterns.push_back(std::string(arg.substr(9)));
    } else if (arg.starts_with("--sender=")) {
      opts.filter_patterns.push_back(std::string(arg.substr(9)));
    } else if (arg == "-r" || arg == "--reverse") {
      opts.reverse_order = true;
    } else if (arg.starts_with("-")) {
      if (parse_int64(arg).has_value()) {
        positional_args.push_back(args[i]);
      }
    } else {
      positional_args.push_back(args[i]);
    }
  }

  if (!opts.filter_patterns.empty()) {
    std::string combined;
    for (size_t i = 0; i < opts.filter_patterns.size(); ++i) {
      if (i > 0) {
        combined += "|";
      }
      combined += std::format("({})", opts.filter_patterns[i]);
    }
    try {
      opts.compiled_filter_regex =
          std::regex(combined, std::regex::icase);
      opts.has_filter = true;
    } catch (const std::regex_error &e) {
      return std::unexpected("Invalid filter pattern: " +
                             std::string(e.what()));
    }
  }

  return opts;
}

bool ListOptions::matches_filter(std::string_view text) const {
  if (!has_filter)
    return true;
  return std::regex_search(text.begin(), text.end(), compiled_filter_regex);
}

bool ListOptions::matches_filter_multi(
    const std::vector<std::string_view> &texts) const {
  if (!has_filter)
    return true;
  for (const auto &txt : texts) {
    if (!txt.empty() &&
        std::regex_search(txt.begin(), txt.end(), compiled_filter_regex)) {
      return true;
    }
  }
  return false;
}

bool ListOptions::matches_since(int64_t timestamp) const {
  if (since_timestamp <= 0)
    return true;
  return timestamp >= since_timestamp;
}

} // namespace grm
