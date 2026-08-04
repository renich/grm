#include "grm/app.hpp"
#include <charconv>
#include <format>
#include <iostream>
#include <regex>

namespace grm {

std::expected<int, std::string>
App::cmd_extract_bday(const std::vector<std::string> &args) {
  if (args.empty()) {
    return std::unexpected("Usage: grm extract bday <chat_id>");
  }

  int64_t chat_id = 0;
  auto [ptr, ec] =
      std::from_chars(args[0].data(), args[0].data() + args[0].size(), chat_id);
  if (ec != std::errc{} || ptr != args[0].data() + args[0].size()) {
    return std::unexpected("Invalid chat_id: " + args[0]);
  }

  if (auto res = ensure_authenticated(); !res) {
    return std::unexpected(res.error());
  }

  ensure_chat_loaded(chat_id);

  const std::string payload = std::format(
      R"({{"chat_id": {}, "from_message_id": 0, "offset": 0, "limit": 100}})",
      chat_id);

  auto res = client_->send_request("getChatHistory", payload, 10.0);
  if (!res) {
    return std::unexpected("Failed to get chat history: " + res.error());
  }

  auto msgs = res->get_array("messages");
  const std::regex bday_regex(
      R"((setbirthday|micumple|cumple|\b\d{1,2}[\/\.-]\d{1,2}([\/\.-]\d{2,4})?\b))",
      std::regex::icase);

  std::cout << std::format("Scanning {} messages for birthdays in chat {}\n",
                           msgs.size(), chat_id);
  std::cout << std::string(60, '=') << "\n";

  for (const auto &m : msgs) {
    auto id = m.get_int("id").value_or(0);
    std::string text;
    if (auto content = m.get_object("content")) {
      if (auto text_obj = content->get_object("text")) {
        text = text_obj->get_string("text").value_or("");
      }
    }

    if (!text.empty() && std::regex_search(text, bday_regex)) {
      std::cout << std::format("[MsgID {}]: {}\n", id, text);
    }
  }

  return 0;
}

} // namespace grm
