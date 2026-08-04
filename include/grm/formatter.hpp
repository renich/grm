#pragma once

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace grm::fmt {

enum class OutputFormat { Auto, Human, Markdown, Json, Plain };

enum class ColorMode { Auto, Always, Never };

struct ChatItem {
  int64_t id{0};
  std::string type;
  std::string title;
};

struct TopicItem {
  int64_t id{0};
  std::string name;
  int64_t message_count{0};
};

struct MessageItem {
  int64_t id{0};
  int64_t chat_id{0};
  int64_t date{0};
  std::string sender;
  std::string text;
};

[[nodiscard]] std::string_view humanize_chat_type(std::string_view tdlib_type);
[[nodiscard]] std::string_view
humanize_auth_code_type(std::string_view tdlib_type);

[[nodiscard]] bool should_use_color(ColorMode mode);

class Formatter {
public:
  static void print_chats(const std::vector<ChatItem> &chats,
                          OutputFormat format = OutputFormat::Auto,
                          ColorMode color_mode = ColorMode::Auto);

  static void print_topics(const std::vector<TopicItem> &topics,
                           OutputFormat format = OutputFormat::Auto,
                           ColorMode color_mode = ColorMode::Auto);

  static void print_messages(const std::vector<MessageItem> &messages,
                             OutputFormat format = OutputFormat::Auto,
                             ColorMode color_mode = ColorMode::Auto);
};

} // namespace grm::fmt
