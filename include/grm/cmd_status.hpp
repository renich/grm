#pragma once

#include "grm/command_registry.hpp"
#include <cstdint>
#include <expected>
#include <string>
#include <string_view>
#include <vector>

namespace grm {

struct StatusListOptions {
  std::string filter;
  bool recent{false};
  bool packs{false};
};

struct StatusSetOptions {
  std::string custom_emoji_id;
  int32_t duration_seconds{0}; // 0 = default / permanent
  int64_t chat_id{0};          // 0 for user, or channel/supergroup ID
};

struct StatusClearOptions {
  int64_t chat_id{0}; // 0 for user, or channel/supergroup ID
};

[[nodiscard]] CommandSpec get_status_spec();

[[nodiscard]] bool parse_status_ls_args(const std::vector<std::string> &args,
                                        StatusListOptions &opts,
                                        std::string &err);

[[nodiscard]] std::expected<int32_t, std::string>
parse_duration_string(std::string_view str);

[[nodiscard]] bool parse_status_set_args(const std::vector<std::string> &args,
                                         StatusSetOptions &opts,
                                         std::string &err);

[[nodiscard]] bool parse_status_clear_args(const std::vector<std::string> &args,
                                           StatusClearOptions &opts,
                                           std::string &err);

[[nodiscard]] std::string
build_set_emoji_status_json(const StatusSetOptions &opts,
                            int64_t current_time = 0);

[[nodiscard]] std::string build_clear_emoji_status_json(int64_t chat_id);

} // namespace grm
