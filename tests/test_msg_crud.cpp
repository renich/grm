// TestMsgCrud validates [FUNC-003] (Message Edit & Delete Operations)
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

static std::string build_edit_message_payload(int64_t chat_id,
                                              int64_t message_id,
                                              const std::string &new_text) {
  return std::format(
      R"({{
        "@type": "editMessageText",
        "chat_id": {},
        "message_id": {},
        "input_message_content": {{
          "@type": "inputMessageText",
          "text": {{
            "@type": "formattedText",
            "text": "{}"
          }}
        }}
      }})",
      chat_id, message_id, grm::escape_json_string(new_text));
}

static std::string build_delete_messages_payload(int64_t chat_id,
                                                 int64_t message_id,
                                                 bool revoke) {
  return std::format(
      R"({{
        "@type": "deleteMessages",
        "chat_id": {},
        "message_ids": [{}],
        "revoke": {}
      }})",
      chat_id, message_id, revoke ? "true" : "false");
}

static std::string build_send_message_reply_payload(int64_t chat_id,
                                                    int64_t reply_to_id,
                                                    const std::string &text) {
  return std::format(
      R"({{
        "@type": "sendMessage",
        "chat_id": {},
        "reply_to": {{
          "@type": "inputMessageReplyToMessage",
          "message_id": {}
        }},
        "input_message_content": {{
          "@type": "inputMessageText",
          "text": {{
            "@type": "formattedText",
            "text": "{}"
          }}
        }}
      }})",
      chat_id, reply_to_id, grm::escape_json_string(text));
}

} // namespace grm::test

void test_edit_message_payload() {
  std::string payload = grm::test::build_edit_message_payload(
      -1001789902965, 3145728, "Updated text message content");
  check(payload.find("editMessageText") != std::string::npos, "Type match");
  check(payload.find("-1001789902965") != std::string::npos, "Chat ID match");
  check(payload.find("3145728") != std::string::npos, "Msg ID match");
  check(payload.find("Updated text message content") != std::string::npos,
        "Text match");
  std::cout << "[PASS] test_edit_message_payload\n";
}

void test_delete_messages_payload() {
  std::string payload =
      grm::test::build_delete_messages_payload(-1001789902965, 3145728, true);
  check(payload.find("deleteMessages") != std::string::npos, "Type match");
  check(payload.find("\"revoke\": true") != std::string::npos, "Revoke match");
  std::cout << "[PASS] test_delete_messages_payload\n";
}

void test_send_reply_to_payload() {
  std::string payload = grm::test::build_send_message_reply_payload(
      -1001789902965, 42, "Replying to message");
  check(payload.find("sendMessage") != std::string::npos, "Type match");
  check(payload.find("inputMessageReplyToMessage") != std::string::npos,
        "Reply-to type match");
  check(payload.find("\"message_id\": 42") != std::string::npos,
        "Reply-to msg_id match");
  std::cout << "[PASS] test_send_reply_to_payload\n";
}

int main() {
  std::cout << "Running Message CRUD unit tests...\n";
  test_edit_message_payload();
  test_delete_messages_payload();
  test_send_reply_to_payload();
  std::cout << "All Message CRUD unit tests passed successfully.\n";
  return 0;
}
