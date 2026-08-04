#pragma once

#include <expected>
#include <filesystem>
#include <string>

namespace grm {

class Downloader {
public:
  [[nodiscard]] static std::expected<std::string, std::string>
  build_download_file_payload(int32_t file_id, int priority = 1);

  [[nodiscard]] static std::expected<std::filesystem::path, std::string>
  resolve_destination(const std::filesystem::path &output_dir,
                      const std::string &file_name);

  [[nodiscard]] static bool
  matches_file_type(const std::string &tdlib_content_type,
                    const std::string &requested_type);
};

} // namespace grm
