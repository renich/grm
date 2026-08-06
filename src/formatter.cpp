#include "grm/formatter.hpp"
#include "grm/json_utils.hpp"

#include <chrono>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <format>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <unistd.h>

namespace grm::fmt {

namespace {

// ANSI escape sequences
constexpr std::string_view COLOR_RESET = "\033[0m";
constexpr std::string_view COLOR_BOLD = "\033[1m";
constexpr std::string_view COLOR_DIM = "\033[2m";
constexpr std::string_view COLOR_CYAN = "\033[36m";
constexpr std::string_view COLOR_GREEN = "\033[32m";
constexpr std::string_view COLOR_YELLOW = "\033[33m";
constexpr std::string_view COLOR_RED = "\033[31m";
constexpr std::string_view COLOR_GRAY = "\033[90m";

} // namespace

std::string_view humanize_chat_type(std::string_view tdlib_type) {
  if (tdlib_type == "chatTypeSupergroup")
    return "Supergroup";
  if (tdlib_type == "chatTypeBasicGroup")
    return "Basic Group";
  if (tdlib_type == "chatTypePrivate")
    return "Private Chat";
  if (tdlib_type == "chatTypeSecret")
    return "Secret Chat";

  if (tdlib_type.starts_with("chatType")) {
    return tdlib_type.substr(8);
  }
  return tdlib_type;
}

std::string_view humanize_auth_code_type(std::string_view tdlib_type) {
  if (tdlib_type == "authenticationCodeTypeTelegramMessage")
    return "In-App Message";
  if (tdlib_type == "authenticationCodeTypeSms")
    return "SMS";
  if (tdlib_type == "authenticationCodeTypeCall")
    return "Phone Call";

  if (tdlib_type.starts_with("authenticationCodeType")) {
    return tdlib_type.substr(22);
  }
  return tdlib_type;
}

std::string humanize_bytes(int64_t bytes) {
  if (bytes < 0)
    return "0 B";
  if (bytes < 1024)
    return std::format("{} B", bytes);
  constexpr double kKiB = 1024.0;
  constexpr double kMiB = 1024.0 * 1024.0;
  constexpr double kGiB = 1024.0 * 1024.0 * 1024.0;

  auto val = static_cast<double>(bytes);
  if (val < kMiB) {
    return std::format("{:.1f} KiB", val / kKiB);
  }
  if (val < kGiB) {
    return std::format("{:.1f} MiB", val / kMiB);
  }
  return std::format("{:.2f} GiB", val / kGiB);
}

std::string humanize_relative_time(int64_t timestamp_sec, int64_t now_sec) {
  if (timestamp_sec <= 0)
    return "Never";
  if (now_sec <= 0) {
    now_sec = std::chrono::duration_cast<std::chrono::seconds>(
                  std::chrono::system_clock::now().time_since_epoch())
                  .count();
  }

  int64_t diff = now_sec - timestamp_sec;
  if (diff < -5)
    return "In the future";
  if (diff < 30)
    return "Just now";
  if (diff < 60)
    return std::format("{}s ago", diff);
  if (diff < 3600)
    return std::format("{}m ago", diff / 60);
  if (diff < 86400)
    return std::format("{}h ago", diff / 3600);
  if (diff < 604800)
    return std::format("{}d ago", diff / 86400);

  return format_iso8601(timestamp_sec);
}

std::string format_iso8601(int64_t timestamp_sec) {
  if (timestamp_sec <= 0)
    return "";
  auto t = static_cast<std::time_t>(timestamp_sec);
  std::tm tm_buf{};
#if defined(_POSIX_C_SOURCE) || defined(_GNU_SOURCE)
  gmtime_r(&t, &tm_buf);
#else
  tm_buf = *std::gmtime(&t);
#endif
  std::ostringstream ss;
  ss << std::put_time(&tm_buf, "%Y-%m-%dT%H:%M:%SZ");
  return ss.str();
}

bool should_use_color(ColorMode mode) {
  if (mode == ColorMode::Never) {
    return false;
  }
  if (mode == ColorMode::Always) {
    return true;
  }

  const char *no_color = std::getenv("NO_COLOR");
  if (no_color && no_color[0] != '\0') {
    return false;
  }

  return isatty(STDOUT_FILENO) != 0;
}

OutputFormat resolve_format(OutputFormat requested_format, bool is_tty_stream) {
  if (requested_format == OutputFormat::Auto) {
    return is_tty_stream ? OutputFormat::Human : OutputFormat::Plain;
  }
  return requested_format;
}

void Formatter::print_chats(const std::vector<ChatItem> &chats,
                            OutputFormat format, ColorMode color_mode,
                            std::ostream &out) {
  const bool is_tty =
      (out.rdbuf() == std::cout.rdbuf() && isatty(STDOUT_FILENO) != 0);
  const OutputFormat fmt = resolve_format(format, is_tty);
  const bool use_color = should_use_color(color_mode);

  if (fmt == OutputFormat::Json) {
    out << "{\n  \"status\": \"success\",\n  \"count\": " << chats.size()
        << ",\n  \"data\": [\n";
    for (size_t i = 0; i < chats.size(); ++i) {
      const auto &c = chats[i];
      out << std::format("    {{\n"
                         "      \"id\": {},\n"
                         "      \"type\": \"{}\",\n"
                         "      \"title\": \"{}\",\n"
                         "      \"unread_count\": {},\n"
                         "      \"last_message_date\": {},\n"
                         "      \"last_message_iso\": \"{}\"\n"
                         "    }}{}",
                         c.id, escape_json_string(humanize_chat_type(c.type)),
                         escape_json_string(c.title), c.unread_count,
                         c.last_message_date,
                         format_iso8601(c.last_message_date),
                         (i + 1 < chats.size() ? ",\n" : "\n"));
    }
    out << "  ]\n}\n";
    return;
  }

  if (fmt == OutputFormat::JsonL) {
    for (const auto &c : chats) {
      out << std::format(
          R"({{"id":{},"type":"{}","title":"{}","unread_count":{},"last_message_date":{}}})"
          "\n",
          c.id, escape_json_string(humanize_chat_type(c.type)),
          escape_json_string(c.title), c.unread_count, c.last_message_date);
    }
    return;
  }

  if (fmt == OutputFormat::Markdown) {
    out << "| Chat ID | Type | Title | Unread |\n";
    out << "| :--- | :--- | :--- | :--- |\n";
    for (const auto &c : chats) {
      out << std::format("| {} | {} | {} | {} |\n", c.id,
                         humanize_chat_type(c.type), c.title, c.unread_count);
    }
    return;
  }

  // Human / Plain mode
  if (use_color) {
    out << std::format("{}{:<20} {:<15} {:<30} {}{}\n", COLOR_BOLD, "CHAT ID",
                       "TYPE", "TITLE", "UNREAD", COLOR_RESET);
    out << COLOR_GRAY << std::string(75, '-') << COLOR_RESET << "\n";
  } else {
    out << std::format("{:<20} {:<15} {:<30} {}\n", "CHAT ID", "TYPE", "TITLE",
                       "UNREAD");
    out << std::string(75, '-') << "\n";
  }

  for (const auto &c : chats) {
    const auto human_type = humanize_chat_type(c.type);
    if (use_color) {
      out << std::format("{}{:<20}{} {}{:<15}{} {}{:<30}{} {}{}{}\n",
                         COLOR_CYAN, c.id, COLOR_RESET, COLOR_YELLOW,
                         human_type, COLOR_RESET, COLOR_GREEN, c.title,
                         COLOR_RESET, COLOR_RED, c.unread_count, COLOR_RESET);
    } else {
      out << std::format("{:<20} {:<15} {:<30} {}\n", c.id, human_type, c.title,
                         c.unread_count);
    }
  }
}

void Formatter::print_topics(const std::vector<TopicItem> &topics,
                             OutputFormat format, ColorMode color_mode,
                             std::ostream &out) {
  const bool is_tty =
      (out.rdbuf() == std::cout.rdbuf() && isatty(STDOUT_FILENO) != 0);
  const OutputFormat fmt = resolve_format(format, is_tty);
  const bool use_color = should_use_color(color_mode);

  if (fmt == OutputFormat::Json) {
    out << "{\n  \"status\": \"success\",\n  \"count\": " << topics.size()
        << ",\n  \"data\": [\n";
    for (size_t i = 0; i < topics.size(); ++i) {
      const auto &t = topics[i];
      out << std::format("    {{\n"
                         "      \"id\": {},\n"
                         "      \"name\": \"{}\",\n"
                         "      \"messages_count\": {},\n"
                         "      \"custom_emoji_id\": {}\n"
                         "    }}{}",
                         t.id, escape_json_string(t.name), t.message_count,
                         t.custom_emoji_id,
                         (i + 1 < topics.size() ? ",\n" : "\n"));
    }
    out << "  ]\n}\n";
    return;
  }

  if (fmt == OutputFormat::JsonL) {
    for (const auto &t : topics) {
      out << std::format(
          R"({{"id":{},"name":"{}","messages_count":{},"custom_emoji_id":{}}})"
          "\n",
          t.id, escape_json_string(t.name), t.message_count, t.custom_emoji_id);
    }
    return;
  }

  if (fmt == OutputFormat::Markdown) {
    out << "| Topic ID | Name | Message Count |\n";
    out << "| :--- | :--- | :--- |\n";
    for (const auto &t : topics) {
      out << std::format("| {} | {} | {} |\n", t.id, t.name, t.message_count);
    }
    return;
  }

  // Human / Plain mode
  if (use_color) {
    out << std::format("{}{:<15} {:<30} {}{}\n", COLOR_BOLD, "TOPIC ID", "NAME",
                       "MESSAGES COUNT", COLOR_RESET);
    out << COLOR_GRAY << std::string(65, '-') << COLOR_RESET << "\n";
  } else {
    out << std::format("{:<15} {:<30} {}\n", "TOPIC ID", "NAME",
                       "MESSAGES COUNT");
    out << std::string(65, '-') << "\n";
  }

  for (const auto &t : topics) {
    if (use_color) {
      out << std::format("{}{:<15}{} {}{:<30}{} {}{}{}\n", COLOR_CYAN, t.id,
                         COLOR_RESET, COLOR_GREEN, t.name, COLOR_RESET,
                         COLOR_YELLOW, t.message_count, COLOR_RESET);
    } else {
      out << std::format("{:<15} {:<30} {}\n", t.id, t.name, t.message_count);
    }
  }
}

static std::vector<std::string_view> split_lines(std::string_view text) {
  std::vector<std::string_view> lines;
  size_t start = 0;
  size_t pos = 0;
  while ((pos = text.find('\n', start)) != std::string_view::npos) {
    lines.push_back(text.substr(start, pos - start));
    start = pos + 1;
  }
  lines.push_back(text.substr(start));
  return lines;
}

static std::string escape_markdown_table_cell(std::string_view text) {
  std::string escaped;
  escaped.reserve(text.size() * 2);
  for (char c : text) {
    if (c == '\n') {
      escaped += "<br>";
    } else if (c == '|') {
      escaped += "\\|";
    } else {
      escaped += c;
    }
  }
  return escaped;
}

void Formatter::print_messages(const std::vector<MessageItem> &messages,
                               OutputFormat format, ColorMode color_mode,
                               std::ostream &out) {
  const bool is_tty =
      (out.rdbuf() == std::cout.rdbuf() && isatty(STDOUT_FILENO) != 0);
  const OutputFormat fmt = resolve_format(format, is_tty);
  const bool use_color = should_use_color(color_mode);

  if (fmt == OutputFormat::Json) {
    out << "{\n  \"status\": \"success\",\n  \"count\": " << messages.size()
        << ",\n  \"data\": [\n";
    for (size_t i = 0; i < messages.size(); ++i) {
      const auto &m = messages[i];
      out << std::format("    {{\n"
                         "      \"id\": {},\n"
                         "      \"chat_id\": {},\n"
                         "      \"date\": {},\n"
                         "      \"date_iso\": \"{}\",\n"
                         "      \"sender\": \"{}\",\n"
                         "      \"text\": \"{}\",\n"
                         "      \"has_attachment\": {},\n"
                         "      \"attachment_type\": \"{}\"\n"
                         "    }}{}",
                         m.id, m.chat_id, m.date, format_iso8601(m.date),
                         escape_json_string(m.sender),
                         escape_json_string(m.text),
                         (m.has_attachment ? "true" : "false"),
                         escape_json_string(m.attachment_type),
                         (i + 1 < messages.size() ? ",\n" : "\n"));
    }
    out << "  ]\n}\n";
    return;
  }

  if (fmt == OutputFormat::JsonL) {
    for (const auto &m : messages) {
      out << std::format(
          R"({{"id":{},"chat_id":{},"date":{},"date_iso":"{}","sender":"{}","text":"{}","has_attachment":{},"attachment_type":"{}"}})"
          "\n",
          m.id, m.chat_id, m.date, format_iso8601(m.date),
          escape_json_string(m.sender), escape_json_string(m.text),
          (m.has_attachment ? "true" : "false"),
          escape_json_string(m.attachment_type));
    }
    return;
  }

  if (fmt == OutputFormat::Markdown) {
    out << "| Message ID | Date | Sender | Text |\n";
    out << "| :--- | :--- | :--- | :--- |\n";
    for (const auto &m : messages) {
      out << std::format("| {} | {} | {} | {} |\n", m.id,
                         format_iso8601(m.date), m.sender,
                         escape_markdown_table_cell(m.text));
    }
    return;
  }

  // Human / Plain mode
  for (const auto &m : messages) {
    auto lines = split_lines(m.text);
    if (lines.empty()) {
      continue;
    }

    const std::string date_str = humanize_relative_time(m.date);
    std::string sender_part;
    if (!m.sender.empty()) {
      sender_part = std::format("<{}> ", m.sender);
    }
    const std::string prefix_plain =
        std::format("[MsgID {} | {}] {}: ", m.id, date_str, sender_part);
    const std::string indent(prefix_plain.size(), ' ');

    if (use_color) {
      out << std::format("{}[MsgID {}]{} {}({}){} {}{}{}: {}\n", COLOR_CYAN, m.id,
                         COLOR_RESET, COLOR_DIM, date_str, COLOR_RESET,
                         COLOR_YELLOW, sender_part, COLOR_RESET,
                         lines[0]);
      for (size_t i = 1; i < lines.size(); ++i) {
        out << std::format("{}{}\n", indent, lines[i]);
      }
    } else {
      out << std::format("[MsgID {} | {}] {}{}\n", m.id, date_str, sender_part, lines[0]);
      for (size_t i = 1; i < lines.size(); ++i) {
        out << std::format("{}{}\n", indent, lines[i]);
      }
    }
  }
}

void Formatter::print_error(const ErrorPayload &err, OutputFormat format,
                            ColorMode color_mode, std::ostream &out) {
  const bool is_tty =
      (out.rdbuf() == std::cerr.rdbuf() && isatty(STDERR_FILENO) != 0);
  const OutputFormat fmt = resolve_format(format, is_tty);
  const bool use_color = should_use_color(color_mode);

  if (fmt == OutputFormat::Json || fmt == OutputFormat::JsonL) {
    out << std::format("{{\n"
                       "  \"status\": \"error\",\n"
                       "  \"code\": {},\n"
                       "  \"error_type\": \"{}\",\n"
                       "  \"message\": \"{}\",\n"
                       "  \"remediation\": \"{}\"\n"
                       "}}\n",
                       err.code, escape_json_string(err.error_type),
                       escape_json_string(err.message),
                       escape_json_string(err.remediation));
    return;
  }

  if (use_color) {
    out << std::format("{}[ERROR {}]{} {}: {}\n", COLOR_RED, err.code,
                       COLOR_RESET, err.error_type, err.message);
    if (!err.remediation.empty()) {
      out << std::format("{}[TIP]{} {}\n", COLOR_YELLOW, COLOR_RESET,
                         err.remediation);
    }
  } else {
    out << std::format("[ERROR {}] {}: {}\n", err.code, err.error_type,
                       err.message);
    if (!err.remediation.empty()) {
      out << std::format("[TIP] {}\n", err.remediation);
    }
  }
}

void Formatter::render(const RenderablePayload &payload,
                       std::string_view /*command_name*/, OutputFormat format,
                       ColorMode color_mode, std::ostream &out) {
  std::visit(
      [&](const auto &data) {
        using T = std::decay_t<decltype(data)>;
        if constexpr (std::is_same_v<T, std::vector<ChatItem>>) {
          print_chats(data, format, color_mode, out);
        } else if constexpr (std::is_same_v<T, std::vector<TopicItem>>) {
          print_topics(data, format, color_mode, out);
        } else if constexpr (std::is_same_v<T, std::vector<MessageItem>>) {
          print_messages(data, format, color_mode, out);
        } else if constexpr (std::is_same_v<T, ErrorPayload>) {
          print_error(data, format, color_mode, out);
        }
      },
      payload);
}

} // namespace grm::fmt
