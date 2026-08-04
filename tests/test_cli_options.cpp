#include <cassert>
#include <charconv>
#include <iostream>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace grm::test {

static std::optional<int>
parse_limit(const std::vector<std::string_view> &args) {
  for (size_t i = 0; i < args.size(); ++i) {
    if (args[i] == "-n" && i + 1 < args.size()) {
      int val = 0;
      auto [ptr, ec] = std::from_chars(
          args[i + 1].data(), args[i + 1].data() + args[i + 1].size(), val);
      if (ec == std::errc{})
        return val;
    } else if (args[i].starts_with("--limit=")) {
      auto val_str = args[i].substr(8);
      int val = 0;
      auto [ptr, ec] = std::from_chars(val_str.data(),
                                       val_str.data() + val_str.size(), val);
      if (ec == std::errc{})
        return val;
    } else if (args[i] == "--limit" && i + 1 < args.size()) {
      int val = 0;
      auto [ptr, ec] = std::from_chars(
          args[i + 1].data(), args[i + 1].data() + args[i + 1].size(), val);
      if (ec == std::errc{})
        return val;
    }
  }
  return std::nullopt;
}

static std::optional<std::string>
parse_format(const std::vector<std::string_view> &args) {
  for (size_t i = 0; i < args.size(); ++i) {
    if ((args[i] == "-f" || args[i] == "--format") && i + 1 < args.size()) {
      return std::string(args[i + 1]);
    } else if (args[i].starts_with("--format=")) {
      return std::string(args[i].substr(9));
    }
  }
  return std::nullopt;
}

} // namespace grm::test

int main() {
  std::cout << "Running test_cli_options...\n";

  // Test short -n limit flag
  auto limit1 = grm::test::parse_limit({"-n", "50"});
  assert(limit1.has_value() && *limit1 == 50);

  // Test long --limit=100 flag
  auto limit2 = grm::test::parse_limit({"--limit=100"});
  assert(limit2.has_value() && *limit2 == 100);

  // Test long --limit 75 flag
  auto limit3 = grm::test::parse_limit({"--limit", "75"});
  assert(limit3.has_value() && *limit3 == 75);

  // Test format options
  auto fmt1 = grm::test::parse_format({"-f", "csv"});
  assert(fmt1.has_value() && *fmt1 == "csv");

  auto fmt2 = grm::test::parse_format({"--format=json"});
  assert(fmt2.has_value() && *fmt2 == "json");

  std::cout << "test_cli_options passed successfully!\n";
  return 0;
}
