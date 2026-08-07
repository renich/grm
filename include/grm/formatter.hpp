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
  int64_t last_message_date{0};
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

struct ChatFolderSummary {
  int32_t id{0};
  std::string title;
  std::string icon;
  int32_t color_id{-1};
  bool include_groups{false};
  bool include_channels{false};
  bool include_bots{false};
  bool include_contacts{false};
  bool include_non_contacts{false};
  bool exclude_muted{false};
  bool exclude_read{false};
  bool exclude_archived{false};
  std::vector<int64_t> pinned_chat_ids;
  std::vector<int64_t> included_chat_ids;
  std::vector<int64_t> excluded_chat_ids;
};

struct UserItem {
  int64_t id{0};
  std::string first_name;
  std::string last_name;
  std::string username;
  std::string phone_number;
  std::string status;
};

struct SearchSummary {
  std::string query;
  std::vector<ChatItem> chats;
  std::vector<ChatItem> supergroups;
  std::vector<UserItem> users;
  std::vector<MessageItem> messages;
  std::vector<MessageItem> files;
};

struct ErrorPayload {
  int code{1};
  std::string error_type;
  std::string message;
  std::string remediation;
};

using RenderablePayload =
    std::variant<std::vector<ChatItem>, std::vector<TopicItem>,
                 std::vector<MessageItem>, std::vector<ChatFolderSummary>,
                 std::vector<UserItem>, SearchSummary, ErrorPayload>;

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
                          std::ostream &out = std::cout, bool verbose = false);

  static void print_topics(const std::vector<TopicItem> &topics,
                           OutputFormat format = OutputFormat::Auto,
                           ColorMode color_mode = ColorMode::Auto,
                           std::ostream &out = std::cout, bool verbose = false);

  static void print_messages(const std::vector<MessageItem> &messages,
                             OutputFormat format = OutputFormat::Auto,
                             ColorMode color_mode = ColorMode::Auto,
                             std::ostream &out = std::cout,
                             bool verbose = false);

  static void print_folders(const std::vector<ChatFolderSummary> &folders,
                            OutputFormat format = OutputFormat::Auto,
                            ColorMode color_mode = ColorMode::Auto,
                            std::ostream &out = std::cout,
                            bool verbose = false);

  static void print_users(const std::vector<UserItem> &users,
                          OutputFormat format = OutputFormat::Auto,
                          ColorMode color_mode = ColorMode::Auto,
                          std::ostream &out = std::cout,
                          bool verbose = false);

  static void print_search_summary(const SearchSummary &summary,
                                   OutputFormat format = OutputFormat::Auto,
                                   ColorMode color_mode = ColorMode::Auto,
                                   std::ostream &out = std::cout,
                                   bool verbose = false);

  static void print_error(const ErrorPayload &err,

                          OutputFormat format = OutputFormat::Auto,
                          ColorMode color_mode = ColorMode::Auto,
                          std::ostream &out = std::cerr);

  static void render(const RenderablePayload &payload,
                     std::string_view command_name,
                     OutputFormat format = OutputFormat::Auto,
                     ColorMode color_mode = ColorMode::Auto,
                     std::ostream &out = std::cout,
                     bool verbose = false);
};

} // namespace grm::fmt
