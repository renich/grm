// TestFile validates [FUNC-007] and [TECH-004] (File Upload Payload Generation)
#include "grm/uploader.hpp"
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>

static void check(bool condition, const std::string &msg) {
  if (!condition) {
    std::cerr << "Assertion failed: " << msg << std::endl;
    std::exit(1);
  }
}

void test_uploader_validation() {
  std::filesystem::path non_existent = "non_existent_file_xyz.tmp";
  auto res = grm::Uploader::build_send_document_payload(
      -1001789902965, non_existent, "Test caption", 0);
  check(!res.has_value(), "Non-existent file should fail");
  check(res.error().find("File does not exist") != std::string::npos,
        "Error message should mention missing file");
  std::cout << "[PASS] test_uploader_validation\n";
}

void test_uploader_payload_generation() {
  std::filesystem::path dummy_file = "dummy_upload_test.txt";
  {
    std::ofstream out(dummy_file);
    out << "Sample file content for TDLib upload\n";
  }

  auto res = grm::Uploader::build_send_document_payload(
      -1001789902965, dummy_file, "Hola desde grm file upload", 42);
  check(res.has_value(), "Payload generation should succeed");
  const std::string &payload = *res;
  check(payload.find("sendMessage") != std::string::npos, "Has sendMessage");
  check(payload.find("-1001789902965") != std::string::npos, "Has chat_id");
  check(payload.find("inputMessageDocument") != std::string::npos,
        "Has inputMessageDocument");
  check(payload.find("inputFileLocal") != std::string::npos,
        "Has inputFileLocal");
  check(payload.find("dummy_upload_test.txt") != std::string::npos, "Has path");
  check(payload.find("Hola desde grm file upload") != std::string::npos,
        "Has caption");
  check(payload.find("\"message_thread_id\": 42") != std::string::npos,
        "Has thread ID");

  std::filesystem::remove(dummy_file);
  std::cout << "[PASS] test_uploader_payload_generation\n";
}

int main() {
  std::cout << "Running Uploader unit tests...\n";
  test_uploader_validation();
  test_uploader_payload_generation();
  std::cout << "All Uploader tests passed successfully.\n";
  return 0;
}
