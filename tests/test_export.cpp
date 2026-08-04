// TestExport validates [FUNC-006] (Message Export to CSV & JSON)
#include "grm/exporter.hpp"
#include <cassert>
#include <filesystem>
#include <fstream>
#include <iostream>

void test_export_json() {
  std::vector<grm::MessageRecord> msgs = {
      {101, 1001, 1600000000, "Alice", "Hello World"},
      {102, 1001, 1600000005, "Bob", "Hi Alice!"}};

  std::filesystem::path out_file = "test_export_out.json";
  auto res = grm::Exporter::to_json(msgs, out_file);
  assert(res.has_value());
  assert(std::filesystem::exists(out_file));

  std::ifstream infile(out_file);
  std::string content((std::istreambuf_iterator<char>(infile)),
                      std::istreambuf_iterator<char>());
  assert(content.find("Hello World") != std::string::npos);
  assert(content.find("Alice") != std::string::npos);

  std::filesystem::remove(out_file);
  std::cout << "[PASS] test_export_json\n";
}

void test_export_csv() {
  std::vector<grm::MessageRecord> msgs = {
      {101, 1001, 1600000000, "Alice", "Hello World, \"quoted\""},
      {102, 1001, 1600000005, "Bob", "Line1\nLine2"}};

  std::filesystem::path out_file = "test_export_out.csv";
  auto res = grm::Exporter::to_csv(msgs, out_file);
  assert(res.has_value());
  assert(std::filesystem::exists(out_file));

  std::ifstream infile(out_file);
  std::string content((std::istreambuf_iterator<char>(infile)),
                      std::istreambuf_iterator<char>());
  assert(content.find("id,chat_id,date,sender,text") != std::string::npos);
  assert(content.find("Alice") != std::string::npos);

  std::filesystem::remove(out_file);
  std::cout << "[PASS] test_export_csv\n";
}

int main() {
  std::cout << "Running Exporter unit tests...\n";
  test_export_json();
  test_export_csv();
  std::cout << "All Exporter tests passed successfully.\n";
  return 0;
}
