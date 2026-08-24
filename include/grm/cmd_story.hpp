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
  int64_t chat_id{0}; // 0 for personal account, or channel ID
};

struct StoryEditOptions {
  int32_t story_id{0};
  std::string photo_path;
  std::string video_path;
  std::string caption;
  bool has_caption{false};
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

} // namespace grm
