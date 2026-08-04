// TestJsonUtils validates [TECH-002] and [FUNC-003]
#include "grm/json_utils.hpp"
#include <cassert>
#include <iostream>

void test_json_parse_valid() {
  const std::string json = R"({"@type": "updateAuthorizationState", "api_id": 12345, "enabled": true})";
  auto res = grm::JsonValue::parse(json);
  assert(res.has_value());
  assert(res->is_object());
  assert(res->get_type() == "updateAuthorizationState");
  assert(res->get_int("api_id") == 12345);
  assert(res->get_bool("enabled") == true);
  std::cout << "[PASS] test_json_parse_valid\n";
}

void test_json_parse_invalid() {
  const std::string invalid_json = R"({"invalid": json syntax)";
  auto res = grm::JsonValue::parse(invalid_json);
  assert(!res.has_value());
  std::cout << "[PASS] test_json_parse_invalid\n";
}

void test_json_array_handling() {
  const std::string json = R"({"chat_ids": [1001, 1002, 1003]})";
  auto res = grm::JsonValue::parse(json);
  assert(res.has_value());
  auto arr = res->get_array("chat_ids");
  assert(arr.size() == 3);
  assert(arr[0].get_int("") == 1001);
  assert(arr[1].get_int("") == 1002);
  assert(arr[2].get_int("") == 1003);
  std::cout << "[PASS] test_json_array_handling\n";
}

int main() {
  std::cout << "Running JsonValue unit tests...\n";
  test_json_parse_valid();
  test_json_parse_invalid();
  test_json_array_handling();
  std::cout << "All JsonValue tests passed successfully.\n";
  return 0;
}
