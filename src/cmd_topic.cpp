#include "grm/app.hpp"
#include <charconv>
#include <format>
#include <iostream>

namespace grm {

std::expected<int, std::string>
App::cmd_topic_ls(const std::vector<std::string> &args) {
  if (args.empty()) {
    return std::unexpected("Usage: grm topic ls <supergroup_id>");
  }

  int64_t chat_id = 0;
  auto [ptr, ec] =
      std::from_chars(args[0].data(), args[0].data() + args[0].size(), chat_id);
  if (ec != std::errc{} || ptr != args[0].data() + args[0].size()) {
    return std::unexpected("Invalid supergroup_id: " + args[0]);
  }

  if (auto res = ensure_authenticated(); !res) {
    return std::unexpected(res.error());
  }

  ensure_chat_loaded(chat_id);

  const std::string payload = std::format(
      R"({{
        "chat_id": {},
        "query": "",
        "offset_date": 0,
        "offset_message_id": 0,
        "offset_message_thread_id": 0,
        "limit": 100
      }})",
      chat_id);

  auto res = client_->send_request("getForumTopics", payload, 10.0);
  if (!res) {
    return std::unexpected("Failed to get forum topics: " + res.error());
  }

  auto topics = res->get_array("topics");
  std::cout << std::format("{:<15} {:<30} {}\n", "TOPIC ID", "NAME",
                           "MESSAGES COUNT");
  std::cout << std::string(60, '-') << "\n";

  for (const auto &t : topics) {
    if (auto info = t.get_object("info")) {
      auto thread_id = info->get_int("message_thread_id").value_or(0);
      std::string name = info->get_string("name").value_or("General");
      auto total_messages = t.get_int("total_message_count").value_or(0);
      std::cout << std::format("{:<15} {:<30} {}\n", thread_id, name,
                               total_messages);
    }
  }

  return 0;
}

} // namespace grm
