#include "grm/app.hpp"
#include "grm/uploader.hpp"
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
  try {
    chat_id = std::stoll(args[0]);
  } catch (...) {
    return std::unexpected("Invalid chat_id: " + args[0]);
  }

  const std::filesystem::path file_path = args[1];
  std::string caption;

  for (size_t i = 2; i < args.size(); ++i) {
    if (args[i] == "--caption" && i + 1 < args.size()) {
      caption = args[i + 1];
      ++i;
    }
  }

  if (auto res = ensure_authenticated(); !res) {
    return std::unexpected(res.error());
  }

  auto payload_res =
      Uploader::build_send_document_payload(chat_id, file_path, caption, 0);
  if (!payload_res) {
    return std::unexpected(payload_res.error());
  }

  std::cout << std::format("Uploading {} to chat {}...\n", file_path.string(),
                           chat_id);

  auto res = client_->send_request("sendMessage", *payload_res, 30.0);
  if (!res) {
    return std::unexpected("Failed to send file: " + res.error());
  }

  std::cout << "✓ File uploaded and sent successfully!" << std::endl;
  return 0;
}

} // namespace grm
