// TestTopic validates [FUNC-008] and [TECH-005] (Forum Topics & Thread ID Formatting)
#include "grm/json_utils.hpp"
#include "grm/uploader.hpp"
#include <cstdlib>
#include <fstream>
#include <iostream>

static void check(bool condition, const std::string &msg) {
  if (!condition) {
    std::cerr << "Assertion failed: " << msg << std::endl;
    std::exit(1);
  }
}

void test_topic_payload_generation() {
  std::filesystem::path dummy_file = "dummy_topic_test.txt";
  {
    std::ofstream out(dummy_file);
    out << "Sample topic message file\n";
  }

  // Verify build_send_document_payload sets message_thread_id correctly
  auto res = grm::Uploader::build_send_document_payload(
      -1001789902965, dummy_file, "Topic thread test caption", 9999);
  check(res.has_value(), "Uploader payload build should succeed");
  check(res->find("\"message_thread_id\": 9999") != std::string::npos,
        "Payload must include message_thread_id 9999");

  std::filesystem::remove(dummy_file);
  std::cout << "[PASS] test_topic_payload_generation\n";
}

int main() {
  std::cout << "Running Forum Topics unit tests...\n";
  test_topic_payload_generation();
  std::cout << "All Topic tests passed successfully.\n";
  return 0;
}
