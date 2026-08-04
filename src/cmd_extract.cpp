#include "grm/app.hpp"
#include <charconv>
#include <format>
#include <iostream>
#include <regex>

namespace grm {

std::expected<int, std::string>
App::cmd_extract_bday(const std::vector<std::string> &args) {
  int scan_limit = 100;
  int64_t chat_id = 0;
  bool chat_id_set = false;

  for (size_t i = 0; i < args.size(); ++i) {
    std::string_view arg(args[i]);
    if ((arg == "-n" || arg == "--limit") && i + 1 < args.size()) {
      int val = 0;
      auto [ptr, ec] = std::from_chars(
          args[i + 1].data(), args[i + 1].data() + args[i + 1].size(), val);
      if (ec == std::errc{}) {
        scan_limit = val;
        ++i;
      }
    } else if (arg.starts_with("--limit=")) {
      auto val_str = arg.substr(8);
      int val = 0;
      auto [ptr, ec] = std::from_chars(val_str.data(),
                                       val_str.data() + val_str.size(), val);
      if (ec == std::errc{}) {
        scan_limit = val;
      }
    } else if (!chat_id_set && !arg.starts_with("-")) {
      auto [ptr, ec] =
          std::from_chars(arg.data(), arg.data() + arg.size(), chat_id);
      if (ec == std::errc{} && ptr == arg.data() + arg.size()) {
        chat_id_set = true;
      }
    }
  }

  if (!chat_id_set) {
    return std::unexpected(
        "Usage: grm extract bday <chat_id> [-n|--limit <N>]");
  }


  if (auto res = ensure_authenticated(); !res) {
    return std::unexpected(res.error());
  }

  ensure_chat_loaded(chat_id);

  const std::string payload = std::format(
      R"({{"chat_id": {}, "from_message_id": 0, "offset": 0, "limit": {}}})",
      chat_id, scan_limit);


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
