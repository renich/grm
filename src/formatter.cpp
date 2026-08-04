#include "grm/formatter.hpp"
#include "grm/json_utils.hpp"

#include <cstdlib>
#include <format>
#include <iostream>
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

static OutputFormat resolve_format(OutputFormat requested_format) {
  if (requested_format == OutputFormat::Auto) {
    return (isatty(STDOUT_FILENO) != 0) ? OutputFormat::Human
                                        : OutputFormat::Plain;
  }
  return requested_format;
}

void Formatter::print_chats(const std::vector<ChatItem> &chats,
                            OutputFormat format, ColorMode color_mode) {
  const OutputFormat fmt = resolve_format(format);
  const bool use_color = should_use_color(color_mode);

  if (fmt == OutputFormat::Json) {
    std::cout << "[\n";
    for (size_t i = 0; i < chats.size(); ++i) {
      const auto &c = chats[i];
      std::cout << std::format(
          "  {{\n"
          "    \"id\": {},\n"
          "    \"type\": \"{}\",\n"
          "    \"title\": \"{}\"\n"
          "  }}{}",
          c.id, escape_json_string(humanize_chat_type(c.type)),
          escape_json_string(c.title), (i + 1 < chats.size() ? ",\n" : "\n"));
    }
    std::cout << "]\n";
    return;
  }

  if (fmt == OutputFormat::Markdown) {
    std::cout << "| Chat ID | Type | Title |\n";
    std::cout << "| :--- | :--- | :--- |\n";
    for (const auto &c : chats) {
      std::cout << std::format("| {} | {} | {} |\n", c.id,
                               humanize_chat_type(c.type), c.title);
    }
    return;
  }

  // Human / Plain mode
  if (use_color) {
    std::cout << std::format("{}{:<20} {:<15} {}{}\n", COLOR_BOLD, "CHAT ID",
                             "TYPE", "TITLE", COLOR_RESET);
    std::cout << COLOR_GRAY << std::string(60, '-') << COLOR_RESET << "\n";
  } else {
    std::cout << std::format("{:<20} {:<15} {}\n", "CHAT ID", "TYPE", "TITLE");
    std::cout << std::string(60, '-') << "\n";
  }

  for (const auto &c : chats) {
    const auto human_type = humanize_chat_type(c.type);
    if (use_color) {
      std::cout << std::format("{}{:<20}{} {}{:<15}{} {}{}{}\n", COLOR_CYAN,
                               c.id, COLOR_RESET, COLOR_YELLOW, human_type,
                               COLOR_RESET, COLOR_GREEN, c.title, COLOR_RESET);
    } else {
      std::cout << std::format("{:<20} {:<15} {}\n", c.id, human_type, c.title);
    }
  }
}

void Formatter::print_topics(const std::vector<TopicItem> &topics,
                            OutputFormat format, ColorMode color_mode) {
  const OutputFormat fmt = resolve_format(format);
  const bool use_color = should_use_color(color_mode);

  if (fmt == OutputFormat::Json) {
    std::cout << "[\n";
    for (size_t i = 0; i < topics.size(); ++i) {
      const auto &t = topics[i];
      std::cout << std::format(
          "  {{\n"
          "    \"id\": {},\n"
          "    \"name\": \"{}\",\n"
          "    \"messages_count\": {}\n"
          "  }}{}",
          t.id, escape_json_string(t.name), t.message_count,
          (i + 1 < topics.size() ? ",\n" : "\n"));
    }
    std::cout << "]\n";
    return;
  }

  if (fmt == OutputFormat::Markdown) {
    std::cout << "| Topic ID | Name | Message Count |\n";
    std::cout << "| :--- | :--- | :--- |\n";
    for (const auto &t : topics) {
      std::cout << std::format("| {} | {} | {} |\n", t.id, t.name,
                               t.message_count);
    }
    return;
  }

  // Human / Plain mode
  if (use_color) {
    std::cout << std::format("{}{:<15} {:<30} {}{}\n", COLOR_BOLD, "TOPIC ID",
                             "NAME", "MESSAGES COUNT", COLOR_RESET);
    std::cout << COLOR_GRAY << std::string(60, '-') << COLOR_RESET << "\n";
  } else {
    std::cout << std::format("{:<15} {:<30} {}\n", "TOPIC ID", "NAME",
                             "MESSAGES COUNT");
    std::cout << std::string(60, '-') << "\n";
  }

  for (const auto &t : topics) {
    if (use_color) {
      std::cout << std::format("{}{:<15}{} {}{:<30}{} {}{}{}\n", COLOR_CYAN,
                               t.id, COLOR_RESET, COLOR_GREEN, t.name,
                               COLOR_RESET, COLOR_YELLOW, t.message_count,
                               COLOR_RESET);
    } else {
      std::cout << std::format("{:<15} {:<30} {}\n", t.id, t.name,
                               t.message_count);
    }
  }
}

void Formatter::print_messages(const std::vector<MessageItem> &messages,
                               OutputFormat format, ColorMode color_mode) {
  const OutputFormat fmt = resolve_format(format);
  const bool use_color = should_use_color(color_mode);

  if (fmt == OutputFormat::Json) {
    std::cout << "[\n";
    for (size_t i = 0; i < messages.size(); ++i) {
      const auto &m = messages[i];
      std::cout << std::format(
          "  {{\n"
          "    \"id\": {},\n"
          "    \"chat_id\": {},\n"
          "    \"date\": {},\n"
          "    \"sender\": \"{}\",\n"
          "    \"text\": \"{}\"\n"
          "  }}{}",
          m.id, m.chat_id, m.date, escape_json_string(m.sender),
          escape_json_string(m.text),
          (i + 1 < messages.size() ? ",\n" : "\n"));
    }
    std::cout << "]\n";
    return;
  }

  if (fmt == OutputFormat::Markdown) {
    std::cout << "| Message ID | Sender | Text |\n";
    std::cout << "| :--- | :--- | :--- |\n";
    for (const auto &m : messages) {
      std::cout << std::format("| {} | {} | {} |\n", m.id, m.sender, m.text);
    }
    return;
  }

  // Human / Plain mode
  for (const auto &m : messages) {
    if (use_color) {
      std::cout << std::format("{}[MsgID {}]{}: {}\n", COLOR_CYAN, m.id,
                               COLOR_RESET, m.text);
    } else {
      std::cout << std::format("[MsgID {}]: {}\n", m.id, m.text);
    }
  }
}

} // namespace grm::fmt
