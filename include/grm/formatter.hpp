#pragma once

#include <cstdint>
#include <iostream>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

namespace grm::fmt {

enum class OutputFormat : std::uint8_t {
  Auto,
  Human,
  Markdown,
  Json,
  JsonL,
  Plain
};

enum class ColorMode : std::uint8_t { Auto, Always, Never };

struct ChatItem {
  int64_t id{0};
  std::string type;
  std::string title;
  int32_t unread_count{0};
  int64_t last_message_date{0};
};

struct TopicItem {
  int64_t id{0};
  std::string name;
  int64_t message_count{0};
  int64_t custom_emoji_id{0};
  int32_t icon_color{0};
};

struct MessageItem {
  int64_t id{0};
  int64_t chat_id{0};
  int64_t topic_id{0};
  int64_t date{0};
  std::string sender;
  std::string text;
  bool has_attachment{false};
  std::string attachment_type;
};

struct ErrorPayload {
  int code{1};
  std::string error_type;
  std::string message;
  std::string remediation;
};

using RenderablePayload =
    std::variant<std::vector<ChatItem>, std::vector<TopicItem>,
                 std::vector<MessageItem>, ErrorPayload>;

[[nodiscard]] std::string_view humanize_chat_type(std::string_view tdlib_type);
[[nodiscard]] std::string_view
humanize_auth_code_type(std::string_view tdlib_type);
[[nodiscard]] std::string humanize_bytes(int64_t bytes);
[[nodiscard]] std::string humanize_relative_time(int64_t timestamp_sec,
                                                 int64_t now_sec = 0);
[[nodiscard]] std::string format_iso8601(int64_t timestamp_sec);

[[nodiscard]] bool should_use_color(ColorMode mode);
[[nodiscard]] OutputFormat resolve_format(OutputFormat requested_format,
                                          bool is_tty_stream = false);

class Formatter {
public:
  static void print_chats(const std::vector<ChatItem> &chats,
                          OutputFormat format = OutputFormat::Auto,
                          ColorMode color_mode = ColorMode::Auto,
                          std::ostream &out = std::cout);

  static void print_topics(const std::vector<TopicItem> &topics,
                           OutputFormat format = OutputFormat::Auto,
                           ColorMode color_mode = ColorMode::Auto,
                           std::ostream &out = std::cout);

  static void print_messages(const std::vector<MessageItem> &messages,
                             OutputFormat format = OutputFormat::Auto,
                             ColorMode color_mode = ColorMode::Auto,
                             std::ostream &out = std::cout);

  static void print_error(const ErrorPayload &err,
                          OutputFormat format = OutputFormat::Auto,
                          ColorMode color_mode = ColorMode::Auto,
                          std::ostream &out = std::cerr);

  static void render(const RenderablePayload &payload,
                     std::string_view command_name,
                     OutputFormat format = OutputFormat::Auto,
                     ColorMode color_mode = ColorMode::Auto,
                     std::ostream &out = std::cout);
};

} // namespace grm::fmt
