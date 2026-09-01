#pragma once

#include "grm/command_registry.hpp"
#include <cstdint>
#include <expected>
#include <string>
#include <string_view>
#include <vector>

namespace grm {

struct StoryPostOptions {
  std::string photo_path;
  std::string video_path;
  std::string caption;
  std::string privacy{"everyone"}; // everyone, contacts, close_friends
  int32_t active_period{86400};    // default 24 hours (86400)
  bool is_pinned{false};           // is_posted_to_chat_page
  bool protect_content{false};
  std::string link_url;
  std::string reaction_emoji;
  int64_t chat_id{0}; // 0 for personal account, or channel ID
};

struct StoryEditOptions {
  int32_t story_id{0};
  std::string photo_path;
  std::string video_path;
  std::string caption;
  bool has_caption{false};
  std::string link_url;
  std::string reaction_emoji;
  bool has_areas{false};
  int64_t chat_id{0};
};

struct StoryListOptions {
  int64_t chat_id{0};
  int32_t limit{20};
  bool pinned{false};
  bool archived{false};
  bool all{false};
};

struct StoryDeleteOptions {
  int32_t story_id{0};
  int64_t chat_id{0};
};

struct StoryInfoOptions {
  int32_t story_id{0};
  int64_t chat_id{0};
};

struct StoryViewersOptions {
  int32_t story_id{0};
  int64_t chat_id{0};
  int32_t limit{50};
  std::string query;
};

struct StoryPinOptions {
  int32_t story_id{0};
  int64_t chat_id{0};
  bool is_pinned{true};
};

struct StoryReactOptions {
  int32_t story_id{0};
  std::string emoji;
  int64_t chat_id{0};
};

struct StoryPrivacyOptions {
  int32_t story_id{0};
  std::string privacy{"everyone"};
  int64_t chat_id{0};
};

[[nodiscard]] CommandSpec get_story_spec();

[[nodiscard]] std::expected<int32_t, std::string>
parse_period_string(std::string_view str);

[[nodiscard]] bool parse_story_post_args(const std::vector<std::string> &args,
                                         StoryPostOptions &opts,
                                         std::string &err);

[[nodiscard]] bool parse_story_edit_args(const std::vector<std::string> &args,
                                         StoryEditOptions &opts,
                                         std::string &err);

[[nodiscard]] bool parse_story_ls_args(const std::vector<std::string> &args,
                                       StoryListOptions &opts,
                                       std::string &err);

[[nodiscard]] bool parse_story_delete_args(const std::vector<std::string> &args,
                                           StoryDeleteOptions &opts,
                                           std::string &err);

[[nodiscard]] bool parse_story_info_args(const std::vector<std::string> &args,
                                         StoryInfoOptions &opts,
                                         std::string &err);

[[nodiscard]] bool
parse_story_viewers_args(const std::vector<std::string> &args,
                         StoryViewersOptions &opts, std::string &err);

[[nodiscard]] bool parse_story_pin_args(const std::vector<std::string> &args,
                                        StoryPinOptions &opts, std::string &err,
                                        bool default_pinned = true);

[[nodiscard]] bool parse_story_react_args(const std::vector<std::string> &args,
                                          StoryReactOptions &opts,
                                          std::string &err);

[[nodiscard]] bool
parse_story_privacy_args(const std::vector<std::string> &args,
                         StoryPrivacyOptions &opts, std::string &err);

[[nodiscard]] std::string
build_post_story_json(const StoryPostOptions &opts, int64_t resolved_chat_id,
                      const std::string &formatted_caption_json);

[[nodiscard]] std::string
build_edit_story_json(int64_t chat_id, int32_t story_id,
                      const StoryEditOptions &opts,
                      const std::string &formatted_caption_json);

[[nodiscard]] std::string build_get_chat_active_stories_json(int64_t chat_id);

[[nodiscard]] std::string build_delete_story_json(int64_t chat_id,
                                                  int32_t story_id);

[[nodiscard]] std::string build_toggle_story_is_posted_to_chat_page_json(
    int64_t chat_id, int32_t story_id, bool is_posted_to_chat_page);

[[nodiscard]] std::string
build_set_story_reaction_json(int64_t chat_id, int32_t story_id,
                              const std::string &emoji);

[[nodiscard]] std::string
build_set_story_privacy_settings_json(int32_t story_id,
                                      const std::string &privacy);

[[nodiscard]] std::string sanitize_single_line_caption(std::string_view text);

[[nodiscard]] std::string truncate_utf8(std::string_view text,
                                        size_t max_chars);

} // namespace grm
