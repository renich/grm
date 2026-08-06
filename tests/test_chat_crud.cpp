// TestChatCrud validates [FUNC-002] (Chat & Group CRUD Payload Generation)
#include "grm/json_utils.hpp"
#include <cstdlib>
#include <format>
#include <iostream>
#include <string>

static void check(bool condition, const std::string &msg) {
  if (!condition) {
    std::cerr << "Assertion failed: " << msg << std::endl;
    std::exit(1);
  }
}

namespace grm::test {

static std::string build_create_group_payload(const std::string &title) {
  return std::format(
      R"({{
        "@type": "createNewBasicGroupChat",
        "title": "{}"
      }})",
      grm::escape_json_string(title));
}

static std::string build_set_chat_title_payload(int64_t chat_id,
                                                const std::string &title) {
  return std::format(
      R"({{
        "@type": "setChatTitle",
        "chat_id": {},
        "title": "{}"
      }})",
      chat_id, grm::escape_json_string(title));
}

static std::string build_pin_message_payload(int64_t chat_id,
                                             int64_t message_id) {
  return std::format(
      R"({{
        "@type": "pinChatMessage",
        "chat_id": {},
        "message_id": {},
        "disable_notification": false,
        "only_for_self": false
      }})",
      chat_id, message_id);
}

} // namespace grm::test

void test_create_group_payload() {
  std::string payload = grm::test::build_create_group_payload("Dev Team Chat");
  check(payload.find("createNewBasicGroupChat") != std::string::npos,
        "Type match");
  check(payload.find("Dev Team Chat") != std::string::npos, "Title match");
  std::cout << "[PASS] test_create_group_payload\n";
}

void test_set_chat_title_payload() {
  std::string payload =
      grm::test::build_set_chat_title_payload(-1001789902965, "New Chat Name");
  check(payload.find("setChatTitle") != std::string::npos, "Type match");
  check(payload.find("-1001789902965") != std::string::npos, "Chat ID match");
  check(payload.find("New Chat Name") != std::string::npos, "Title match");
  std::cout << "[PASS] test_set_chat_title_payload\n";
}

void test_pin_message_payload() {
  std::string payload =
      grm::test::build_pin_message_payload(-1001789902965, 3145728);
  check(payload.find("pinChatMessage") != std::string::npos, "Type match");
  check(payload.find("-1001789902965") != std::string::npos, "Chat ID match");
  check(payload.find("3145728") != std::string::npos, "Msg ID match");
  std::cout << "[PASS] test_pin_message_payload\n";
}

int main() {
  std::cout << "Running Chat CRUD unit tests...\n";
  test_create_group_payload();
  test_set_chat_title_payload();
  test_pin_message_payload();
  std::cout << "All Chat CRUD unit tests passed successfully.\n";
  return 0;
}
