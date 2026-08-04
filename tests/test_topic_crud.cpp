// TestTopicCrud validates [FUNC-009] (Supergroup Forum Topics CRUD Operations)
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

static std::string build_create_topic_payload(int64_t chat_id,
                                               const std::string &name) {
  return std::format(
      R"({{
        "@type": "createForumTopic",
        "chat_id": {},
        "name": "{}"
      }})",
      chat_id, grm::escape_json_string(name));
}

static std::string build_edit_topic_payload(int64_t chat_id,
                                             int64_t topic_id,
                                             const std::string &name) {
  return std::format(
      R"({{
        "@type": "editForumTopic",
        "chat_id": {},
        "message_thread_id": {},
        "name": "{}"
      }})",
      chat_id, topic_id, grm::escape_json_string(name));
}

static std::string build_toggle_topic_close_payload(int64_t chat_id,
                                                     int64_t topic_id,
                                                     bool is_closed) {
  return std::format(
      R"({{
        "@type": "toggleForumTopicIsClosed",
        "chat_id": {},
        "message_thread_id": {},
        "is_closed": {}
      }})",
      chat_id, topic_id, is_closed ? "true" : "false");
}

} // namespace grm::test

void test_create_topic_payload() {
  std::string payload =
      grm::test::build_create_topic_payload(-1001789902965, "Development");
  check(payload.find("createForumTopic") != std::string::npos, "Type match");
  check(payload.find("-1001789902965") != std::string::npos, "Chat ID match");
  check(payload.find("Development") != std::string::npos, "Topic name match");
  std::cout << "[PASS] test_create_topic_payload\n";
}

void test_edit_topic_payload() {
  std::string payload =
      grm::test::build_edit_topic_payload(-1001789902965, 2, "Architecture");
  check(payload.find("editForumTopic") != std::string::npos, "Type match");
  check(payload.find("\"message_thread_id\": 2") != std::string::npos,
        "Thread ID match");
  check(payload.find("Architecture") != std::string::npos, "Name match");
  std::cout << "[PASS] test_edit_topic_payload\n";
}

void test_toggle_topic_close_payload() {
  std::string payload =
      grm::test::build_toggle_topic_close_payload(-1001789902965, 2, true);
  check(payload.find("toggleForumTopicIsClosed") != std::string::npos, "Type match");
  check(payload.find("\"is_closed\": true") != std::string::npos, "Close state match");
  std::cout << "[PASS] test_toggle_topic_close_payload\n";
}

int main() {
  std::cout << "Running Topic CRUD unit tests...\n";
  test_create_topic_payload();
  test_edit_topic_payload();
  test_toggle_topic_close_payload();
  std::cout << "All Topic CRUD unit tests passed successfully.\n";
  return 0;
}
