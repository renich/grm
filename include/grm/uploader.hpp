#pragma once

#include <expected>
#include <filesystem>
#include <string>

namespace grm {

class Uploader {
public:
  [[nodiscard]] static std::expected<std::string, std::string>
  build_send_document_payload(int64_t chat_id,
                              const std::filesystem::path &file_path,
                              const std::string &caption = "",
                              int64_t message_thread_id = 0);
};

} // namespace grm
