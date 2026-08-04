#include "grm/app.hpp"
#include "grm/formatter.hpp"
#include <charconv>

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
  std::vector<fmt::TopicItem> items;
  items.reserve(topics.size());

  for (const auto &t : topics) {
    int64_t thread_id = 0;
    std::string name;
    int64_t total_messages = t.get_int("total_message_count").value_or(0);

    if (auto info = t.get_object("info")) {
      if (auto fid = info->get_int("forum_topic_id")) {
        thread_id = *fid;
      } else if (auto tid = info->get_int("message_thread_id")) {
        thread_id = *tid;
      }
      name = info->get_string("name").value_or("");
    }

    if (thread_id == 0) {
      if (auto fid = t.get_int("forum_topic_id")) {
        thread_id = *fid;
      } else if (auto tid = t.get_int("message_thread_id")) {
        thread_id = *tid;
      }
    }

    if (name.empty()) {
      name = t.get_string("name").value_or("General");
    }

    items.push_back(fmt::TopicItem{.id = thread_id,
                                   .name = name,
                                   .message_count = total_messages});
  }

  fmt::Formatter::print_topics(items, options_.format, options_.color_mode);
  return 0;
}

} // namespace grm
