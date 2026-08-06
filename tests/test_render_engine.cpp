#include "grm/formatter.hpp"
#include <cassert>
#include <iostream>
#include <sstream>
#include <string>

int main() {
  std::cout << "Running test_render_engine...\n";

  using namespace grm::fmt;

  // Test 1: Chat type & Auth code type humanization
  assert(humanize_chat_type("chatTypeSupergroup") == "Supergroup");
  assert(humanize_chat_type("chatTypeBasicGroup") == "Basic Group");
  assert(humanize_chat_type("chatTypePrivate") == "Private Chat");
  assert(humanize_chat_type("chatTypeSecret") == "Secret Chat");
  assert(humanize_chat_type("chatTypeCustom") == "Custom");

  assert(humanize_auth_code_type("authenticationCodeTypeTelegramMessage") ==
         "In-App Message");
  assert(humanize_auth_code_type("authenticationCodeTypeSms") == "SMS");

  // Test 2: Humanize bytes
  assert(humanize_bytes(512) == "512 B");
  assert(humanize_bytes(1024) == "1.0 KiB");
  assert(humanize_bytes(1572864) == "1.5 MiB");
  assert(humanize_bytes(1073741824) == "1.00 GiB");

  // Test 3: ISO-8601 formatting
  int64_t fixed_time = 1785968100; // 2026-08-05T22:15:00Z
  std::string iso_str = format_iso8601(fixed_time);
  assert(!iso_str.empty());
  assert(iso_str.find("2026") != std::string::npos);

  // Test 4: Relative time humanization
  assert(humanize_relative_time(fixed_time, fixed_time + 10) == "Just now");
  assert(humanize_relative_time(fixed_time, fixed_time + 120) == "2m ago");
  assert(humanize_relative_time(fixed_time, fixed_time + 7200) == "2h ago");
  assert(humanize_relative_time(fixed_time, fixed_time + 172800) == "2d ago");

  // Test 5: Format resolution logic
  assert(resolve_format(OutputFormat::Auto, true) == OutputFormat::Human);
  assert(resolve_format(OutputFormat::Auto, false) == OutputFormat::Plain);
  assert(resolve_format(OutputFormat::Json, false) == OutputFormat::Json);

  // Test 6: Chat list rendering in JSON Envelope mode
  std::vector<ChatItem> chats = {
      {101, "chatTypeSupergroup", "DevOps Supergroup", 2, fixed_time},
      {102, "chatTypePrivate", "Alice", 0, fixed_time - 3600}};

  std::ostringstream ss_json;
  Formatter::print_chats(chats, OutputFormat::Json, ColorMode::Never, ss_json);
  std::string json_out = ss_json.str();
  assert(json_out.find("\"status\": \"success\"") != std::string::npos);
  assert(json_out.find("\"count\": 2") != std::string::npos);
  assert(json_out.find("DevOps Supergroup") != std::string::npos);

  // Test 7: Chat list rendering in NDJSON (JsonL) mode
  std::ostringstream ss_jsonl;
  Formatter::print_chats(chats, OutputFormat::JsonL, ColorMode::Never,
                         ss_jsonl);
  std::string jsonl_out = ss_jsonl.str();
  assert(jsonl_out.find("{\"id\":101") != std::string::npos);
  assert(jsonl_out.find("{\"id\":102") != std::string::npos);

  // Test 8: Topic rendering in Markdown mode
  std::vector<TopicItem> topics = {{1, "General", 42, 0, 0},
                                   {2, "Support", 15, 0, 0}};
  std::ostringstream ss_md;
  Formatter::print_topics(topics, OutputFormat::Markdown, ColorMode::Never,
                          ss_md);
  std::string md_out = ss_md.str();
  assert(md_out.find("| Topic ID | Name | Message Count |") !=
         std::string::npos);
  assert(md_out.find("| 1 | General | 42 |") != std::string::npos);

  // Test 9: Error payload rendering in JSON & Human mode
  ErrorPayload err{404, "CHAT_NOT_FOUND", "Chat ID -100999 is invalid",
                   "Check chat ID via grm chat ls"};

  std::ostringstream ss_err_json;
  Formatter::print_error(err, OutputFormat::Json, ColorMode::Never,
                         ss_err_json);
  std::string err_json_out = ss_err_json.str();
  assert(err_json_out.find("\"status\": \"error\"") != std::string::npos);
  assert(err_json_out.find("\"code\": 404") != std::string::npos);
  assert(err_json_out.find("Check chat ID via grm chat ls") !=
         std::string::npos);

  // Test 10: Polymorphic render visitor via std::visit
  RenderablePayload payload = chats;
  std::ostringstream ss_visitor;
  Formatter::render(payload, "chat.ls", OutputFormat::Plain, ColorMode::Never,
                    ss_visitor);
  std::string visitor_out = ss_visitor.str();
  assert(visitor_out.find("DevOps Supergroup") != std::string::npos);

  std::cout << "test_render_engine passed successfully!\n";
  return 0;
}
