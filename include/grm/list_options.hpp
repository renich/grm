#ifndef GRM_LIST_OPTIONS_HPP
#define GRM_LIST_OPTIONS_HPP

#include <expected>
#include <regex>
#include <string>
#include <string_view>
#include <vector>

namespace grm {

struct ListOptions {
  int limit{100};
  int64_t since_timestamp{0};
  std::vector<std::string> filter_patterns;
  bool reverse_order{false};
  int64_t topic_id{0};

  std::regex compiled_filter_regex;
  bool has_filter{false};

  [[nodiscard]] static std::expected<ListOptions, std::string>
  parse(const std::vector<std::string> &args,
        std::vector<std::string> &positional_args);

  [[nodiscard]] bool matches_filter(std::string_view text) const;
  [[nodiscard]] bool
  matches_filter_multi(const std::vector<std::string_view> &texts) const;
  [[nodiscard]] bool matches_since(int64_t timestamp) const;
};

} // namespace grm

#endif // GRM_LIST_OPTIONS_HPP
