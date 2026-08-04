#pragma once

#include <expected>
#include <filesystem>
#include <string>
#include <vector>

namespace grm {

struct MessageRecord {
  int64_t id{0};
  int64_t chat_id{0};
  int64_t date{0};
  std::string sender;
  std::string text;
};

class Exporter {
public:
  [[nodiscard]] static std::expected<void, std::string>
  to_json(const std::vector<MessageRecord> &messages,
          const std::filesystem::path &out_path);

  [[nodiscard]] static std::expected<void, std::string>
  to_csv(const std::vector<MessageRecord> &messages,
         const std::filesystem::path &out_path);

private:
  [[nodiscard]] static std::string escape_csv_field(const std::string &field);
  [[nodiscard]] static std::string escape_json_field(const std::string &field);
};

} // namespace grm
