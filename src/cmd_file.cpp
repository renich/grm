#include "grm/app.hpp"
#include "grm/logger.hpp"
#include "grm/uploader.hpp"
#include <charconv>
#include <filesystem>
#include <format>
#include <iostream>

namespace grm {

std::expected<int, std::string>
App::cmd_send_file(const std::vector<std::string> &args) {
  if (args.size() < 2) {
    return std::unexpected(
        "Usage: grm send file <chat_id> <file_path> [--caption \"<text>\"]");
  }

  int64_t chat_id = 0;
  auto [ptr, ec] =
      std::from_chars(args[0].data(), args[0].data() + args[0].size(), chat_id);
  if (ec != std::errc{} || ptr != args[0].data() + args[0].size()) {
    return std::unexpected("Invalid chat_id: " + args[0]);
  }

  const std::filesystem::path file_path = args[1];
  std::string caption;
  int64_t message_thread_id = 0;

  for (size_t i = 2; i < args.size(); ++i) {
    if ((args[i] == "-C" || args[i] == "--caption") && i + 1 < args.size()) {
      caption = args[i + 1];
      ++i;
    } else if ((args[i] == "-t" || args[i] == "--topic") &&
               i + 1 < args.size()) {
      auto [tptr, tec] = std::from_chars(
          args[i + 1].data(), args[i + 1].data() + args[i + 1].size(),
          message_thread_id);
      if (tec == std::errc{} &&
          tptr == args[i + 1].data() + args[i + 1].size()) {
        // Parsed successfully
      }
      ++i;
    }
  }

  if (auto res = ensure_authenticated(); !res) {
    return std::unexpected(res.error());
  }

  auto payload_res = Uploader::build_send_document_payload(
      chat_id, file_path, caption, message_thread_id);

  if (!payload_res) {
    return std::unexpected(payload_res.error());
  }

  grm::log::info(
      std::format("Uploading {} to chat {}...", file_path.string(), chat_id));

  auto res = client_->send_request("sendMessage", *payload_res, 30.0);
  if (!res) {
    return std::unexpected("Failed to send file: " + res.error());
  }

  grm::log::info("File uploaded and sent successfully.");
  return 0;
}

} // namespace grm
