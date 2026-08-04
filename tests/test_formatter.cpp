#include "grm/formatter.hpp"
#include <cassert>
#include <iostream>

int main() {
  std::cout << "Running test_formatter...\n";

  // Test 1: Chat type humanization
  assert(grm::fmt::humanize_chat_type("chatTypeSupergroup") == "Supergroup");
  assert(grm::fmt::humanize_chat_type("chatTypeBasicGroup") == "Basic Group");
  assert(grm::fmt::humanize_chat_type("chatTypePrivate") == "Private Chat");
  assert(grm::fmt::humanize_chat_type("chatTypeSecret") == "Secret Chat");
  assert(grm::fmt::humanize_chat_type("chatTypeUnknown") == "Unknown");

  // Test 2: Auth code type humanization
  assert(grm::fmt::humanize_auth_code_type(
             "authenticationCodeTypeTelegramMessage") == "In-App Message");
  assert(grm::fmt::humanize_auth_code_type("authenticationCodeTypeSms") ==
         "SMS");
  assert(grm::fmt::humanize_auth_code_type("authenticationCodeTypeCall") ==
         "Phone Call");

  // Test 3: Color mode determination
  assert(!grm::fmt::should_use_color(grm::fmt::ColorMode::Never));
  assert(grm::fmt::should_use_color(grm::fmt::ColorMode::Always));

  std::cout << "test_formatter passed successfully!\n";
  return 0;
}
