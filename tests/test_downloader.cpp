// TestDownloader validates [FUNC-008] (File Attachment Single & Bulk Download Engine)
#include "grm/downloader.hpp"
#include <cstdlib>
#include <filesystem>
#include <iostream>

static void check(bool condition, const std::string &msg) {
  if (!condition) {
    std::cerr << "Assertion failed: " << msg << std::endl;
    std::exit(1);
  }
}

void test_download_file_payload() {
  auto res = grm::Downloader::build_download_file_payload(12345, 1);
  check(res.has_value(), "Download file payload should succeed");
  const std::string &payload = *res;
  check(payload.find("downloadFile") != std::string::npos, "Has downloadFile");
  check(payload.find("\"file_id\": 12345") != std::string::npos, "Has file_id");
  check(payload.find("\"priority\": 1") != std::string::npos, "Has priority");
  std::cout << "[PASS] test_download_file_payload\n";
}

void test_destination_resolution() {
  std::filesystem::path out_dir = "/tmp/grm_test_downloads";
  auto res = grm::Downloader::resolve_destination(out_dir, "report.pdf");
  check(res.has_value(), "Resolution should succeed");
  check(res->string().find("report.pdf") != std::string::npos, "Path contains file name");
  std::cout << "[PASS] test_destination_resolution\n";
}

void test_file_type_filtering() {
  check(grm::Downloader::matches_file_type("messagePhoto", "photo"), "Photo match");
  check(grm::Downloader::matches_file_type("messageVideo", "video"), "Video match");
  check(grm::Downloader::matches_file_type("messageDocument", "doc"), "Doc match");
  check(grm::Downloader::matches_file_type("messagePhoto", "all"), "All match");
  check(!grm::Downloader::matches_file_type("messagePhoto", "video"), "Mismatch check");
  std::cout << "[PASS] test_file_type_filtering\n";
}

int main() {
  std::cout << "Running Downloader unit tests...\n";
  test_download_file_payload();
  test_destination_resolution();
  test_file_type_filtering();
  std::cout << "All Downloader unit tests passed successfully.\n";
  return 0;
}
