#include "grm/uploader.hpp"
#include <format>

namespace grm {

static std::string escape_json_field(const std::string &field) {
  std::string escaped;
  escaped.reserve(field.size() + 8);
  for (char c : field) {
    switch (c) {
    case '"':
      escaped += "\\\"";
      break;
    case '\\':
      escaped += "\\\\";
      break;
    case '\n':
      escaped += "\\n";
      break;
    case '\r':
      escaped += "\\r";
      break;
    case '\t':
      escaped += "\\t";
      break;
    default:
      escaped += c;
      break;
    }
  }
  return escaped;
}

std::expected<std::string, std::string> Uploader::build_send_document_payload(
    int64_t chat_id, const std::filesystem::path &file_path,
    const std::string &caption, int64_t message_thread_id) {
  std::error_code ec;
  if (!std::filesystem::exists(file_path, ec)) {
    return std::unexpected("File does not exist: " + file_path.string());
  }

  if (!std::filesystem::is_regular_file(file_path, ec)) {
    return std::unexpected("Target is not a regular file: " +
                           file_path.string());
  }

  const std::string abs_path =
      std::filesystem::absolute(file_path, ec).string();
  const std::string escaped_path = escape_json_field(abs_path);
  const std::string escaped_caption = escape_json_field(caption);

  const std::string payload = std::format(
      R"({{
        "@type": "sendMessage",
        "chat_id": {},
        "message_thread_id": {},
        "input_message_content": {{
          "@type": "inputMessageDocument",
          "document": {{
            "@type": "inputFileLocal",
            "path": "{}"
          }},
          "caption": {{
            "@type": "formattedText",
            "text": "{}"
          }}
        }}
      }})",
      chat_id, message_thread_id, escaped_path, escaped_caption);

  return payload;
}

} // namespace grm
