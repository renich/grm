#include "grm/downloader.hpp"
#include <format>

namespace grm {

std::expected<std::string, std::string>
Downloader::build_download_file_payload(int32_t file_id, int priority) {
  if (file_id <= 0) {
    return std::unexpected("Invalid file_id: " + std::to_string(file_id));
  }

  std::string payload = std::format(
      R"({{
        "@type": "downloadFile",
        "file_id": {},
        "priority": {},
        "offset": 0,
        "limit": 0,
        "synchronous": true
      }})",
      file_id, priority);

  return payload;
}

std::expected<std::filesystem::path, std::string>
Downloader::resolve_destination(const std::filesystem::path &output_dir,
                                const std::string &file_name) {
  std::error_code ec;
  if (!output_dir.empty() && !std::filesystem::exists(output_dir, ec)) {
    if (!std::filesystem::create_directories(output_dir, ec)) {
      return std::unexpected("Failed to create output directory: " +
                             output_dir.string());
    }
  }

  std::filesystem::path target = output_dir / file_name;
  return target;
}

bool Downloader::matches_file_type(const std::string &tdlib_content_type,
                                   const std::string &requested_type) {
  if (requested_type == "all" || requested_type.empty()) {
    return true;
  }

  if (requested_type == "photo" || requested_type == "image") {
    return tdlib_content_type == "messagePhoto";
  }

  if (requested_type == "video") {
    return tdlib_content_type == "messageVideo" ||
           tdlib_content_type == "messageAnimation";
  }

  if (requested_type == "audio") {
    return tdlib_content_type == "messageAudio" ||
           tdlib_content_type == "messageVoiceNote";
  }

  if (requested_type == "doc" || requested_type == "document") {
    return tdlib_content_type == "messageDocument";
  }

  return false;
}

} // namespace grm
