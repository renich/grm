#include "grm/cmd_status.hpp"
#include "grm/json_utils.hpp"
#include <cstdlib>
#include <iostream>

static void test_assert(bool cond, const char *msg, int line) {
  if (!cond) {
    std::cerr << "Assertion failed at line " << line << ": " << msg << "\n";
    std::abort();
  }
}

#define TEST_ASSERT(cond) test_assert((cond), #cond, __LINE__)

void test_duration_parsing() {
  auto d1 = grm::parse_duration_string("30m");
  TEST_ASSERT(d1.has_value() && *d1 == 1800);

  auto d2 = grm::parse_duration_string("1h");
  TEST_ASSERT(d2.has_value() && *d2 == 3600);

  auto d3 = grm::parse_duration_string("2d");
  TEST_ASSERT(d3.has_value() && *d3 == 172800);

  auto d4 = grm::parse_duration_string("1w");
  TEST_ASSERT(d4.has_value() && *d4 == 604800);

  auto d5 = grm::parse_duration_string("3600");
  TEST_ASSERT(d5.has_value() && *d5 == 3600);

  auto d6 = grm::parse_duration_string("1h30m");
  TEST_ASSERT(d6.has_value() && *d6 == 5400);

  auto d_empty = grm::parse_duration_string("");
  TEST_ASSERT(d_empty.has_value() && *d_empty == 0);

  auto d_invalid = grm::parse_duration_string("invalid");
  TEST_ASSERT(!d_invalid.has_value());

  std::cout << "[PASS] test_duration_parsing\n";
}

void test_status_args_parsing() {
  {
    grm::StatusSetOptions opts;
    std::string err;
    std::vector<std::string> args = {"--emoji",    "5368324170671202288",
                                     "--duration", "2h",
                                     "--chat",     "-10012345"};
    bool ok = grm::parse_status_set_args(args, opts, err);
    TEST_ASSERT(ok);
    TEST_ASSERT(opts.custom_emoji_id == "5368324170671202288");
    TEST_ASSERT(opts.duration_seconds == 7200);
    TEST_ASSERT(opts.chat_id == -10012345);
  }

  {
    // Missing emoji ID
    grm::StatusSetOptions opts;
    std::string err;
    std::vector<std::string> args = {"--duration", "1h"};
    bool ok = grm::parse_status_set_args(args, opts, err);
    TEST_ASSERT(!ok);
    TEST_ASSERT(!err.empty());
  }

  {
    grm::StatusClearOptions opts;
    std::string err;
    std::vector<std::string> args = {"--chat", "-10098765"};
    bool ok = grm::parse_status_clear_args(args, opts, err);
    TEST_ASSERT(ok);
    TEST_ASSERT(opts.chat_id == -10098765);
  }

  {
    grm::StatusListOptions opts;
    std::string err;
    std::vector<std::string> args = {"--recent", "-f", "laptop"};
    bool ok = grm::parse_status_ls_args(args, opts, err);
    TEST_ASSERT(ok);
    TEST_ASSERT(opts.recent);
    TEST_ASSERT(!opts.packs);
    TEST_ASSERT(opts.filter == "laptop");
  }

  {
    grm::StatusListOptions opts;
    std::string err;
    std::vector<std::string> args = {"--packs", "💻"};
    bool ok = grm::parse_status_ls_args(args, opts, err);
    TEST_ASSERT(ok);
    TEST_ASSERT(opts.packs);
    TEST_ASSERT(!opts.recent);
    TEST_ASSERT(opts.filter == "💻");
  }

  std::cout << "[PASS] test_status_args_parsing\n";
}

void test_status_json_builders() {
  {
    // User status with duration
    grm::StatusSetOptions opts;
    opts.custom_emoji_id = "5368324170671202288";
    opts.duration_seconds = 3600;
    opts.chat_id = 0;

    std::string json_str = grm::build_set_emoji_status_json(opts, 1700000000);
    auto parsed = grm::JsonValue::parse(json_str);
    if (!parsed.has_value()) {
      TEST_ASSERT(false);
      return;
    }

    auto emoji_status = parsed->get_object("emoji_status");
    if (!emoji_status.has_value()) {
      TEST_ASSERT(false);
      return;
    }
    TEST_ASSERT(emoji_status->get_string("@type").value_or("") ==
                "emojiStatus");
    TEST_ASSERT(emoji_status->get_int("expiration_date").value_or(0) ==
                1700003600);

    auto type_obj = emoji_status->get_object("type");
    if (!type_obj.has_value()) {
      TEST_ASSERT(false);
      return;
    }
    TEST_ASSERT(type_obj->get_string("@type").value_or("") ==
                "emojiStatusTypeCustomEmoji");
    TEST_ASSERT(type_obj->get_string("custom_emoji_id").value_or("") ==
                "5368324170671202288");
  }

  {
    // Chat status indefinite
    grm::StatusSetOptions opts;
    opts.custom_emoji_id = "9988776655";
    opts.duration_seconds = 0;
    opts.chat_id = -10055555;

    std::string json_str = grm::build_set_emoji_status_json(opts, 1700000000);
    auto parsed = grm::JsonValue::parse(json_str);
    if (!parsed.has_value()) {
      TEST_ASSERT(false);
      return;
    }
    TEST_ASSERT(parsed->get_int("chat_id").value_or(0) == -10055555);

    auto emoji_status = parsed->get_object("emoji_status");
    if (!emoji_status.has_value()) {
      TEST_ASSERT(false);
      return;
    }
    TEST_ASSERT(emoji_status->get_int("expiration_date").value_or(-1) == 0);
  }

  {
    // Clear user status
    std::string json_str = grm::build_clear_emoji_status_json(0);
    auto parsed = grm::JsonValue::parse(json_str);
    TEST_ASSERT(parsed.has_value());
  }

  {
    // Clear chat status
    std::string json_str = grm::build_clear_emoji_status_json(-10099999);
    auto parsed = grm::JsonValue::parse(json_str);
    if (!parsed.has_value()) {
      TEST_ASSERT(false);
      return;
    }
    TEST_ASSERT(parsed->get_int("chat_id").value_or(0) == -10099999);
  }

  std::cout << "[PASS] test_status_json_builders\n";
}

int main() {
  test_duration_parsing();
  test_status_args_parsing();
  test_status_json_builders();
  std::cout << "All Emoji Status unit tests passed successfully!\n";
  return 0;
}
