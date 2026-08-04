#include "grm/app.hpp"
#include <format>
#include <iostream>

namespace grm {

std::expected<int, std::string>
App::cmd_chat_ls([[maybe_unused]] const std::vector<std::string> &args) {
  if (auto res = ensure_authenticated(); !res) {
    return std::unexpected(res.error());
  }

  // Pre-load chats from server / database cache
  auto load_res = client_->send_request("loadChats", R"({"limit": 100})", 5.0);
  if (!load_res) {
    grm::log::debug("loadChats: " + load_res.error());
  }



  // Request chats list
  auto chats_res = client_->send_request("getChats", R"({"limit": 100})", 10.0);
  if (!chats_res) {
    return std::unexpected("Failed to get chats: " + chats_res.error());
  }

  auto chat_ids = chats_res->get_array("chat_ids");
  std::cout << std::format("{:<20} {:<15} {}\n", "CHAT ID", "TYPE", "TITLE");
  std::cout << std::string(60, '-') << "\n";

  for (const auto &id_val : chat_ids) {
    if (auto cid = id_val.as_int64()) {

      const std::string chat_req = std::format(R"({{"chat_id": {}}})", *cid);
      auto chat_info = client_->send_request("getChat", chat_req, 3.0);
      if (chat_info) {
        std::string title = chat_info->get_string("title").value_or("Private");
        std::string type_name = "Chat";
        if (auto type_obj = chat_info->get_object("type")) {
          type_name = type_obj->get_type().value_or("Chat");
        }
        std::cout << std::format("{:<20} {:<15} {}\n", *cid, type_name, title);
      }
    }
  }

  return 0;
}

} // namespace grm
