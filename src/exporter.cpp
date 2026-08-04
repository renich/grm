#include "grm/exporter.hpp"
#include <format>
#include <fstream>
#include <sstream>

namespace grm {

std::string Exporter::escape_csv_field(const std::string &field) {
  bool needs_quotes = false;
  std::string escaped;
  escaped.reserve(field.size() + 8);

  for (char c : field) {
    if (c == '"') {
      needs_quotes = true;
      escaped += "\"\"";
    } else {
      if (c == ',' || c == '\n' || c == '\r') {
        needs_quotes = true;
      }
      escaped += c;
    }
  }

  if (needs_quotes) {
    return "\"" + escaped + "\"";
  }
  return escaped;
}

std::string Exporter::escape_json_field(const std::string &field) {
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
    case '\b':
      escaped += "\\b";
      break;
    case '\f':
      escaped += "\\f";
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

std::expected<void, std::string>
Exporter::to_json(const std::vector<MessageRecord> &messages,
                  const std::filesystem::path &out_path) {
  std::ofstream outfile(out_path, std::ios::trunc);
  if (!outfile.is_open()) {
    return std::unexpected("Failed to open file for JSON export: " +
                           out_path.string());
  }

  outfile << "[\n";
  for (size_t i = 0; i < messages.size(); ++i) {
    const auto &m = messages[i];
    outfile << std::format("  {{\n"
                           "    \"id\": {},\n"
                           "    \"chat_id\": {},\n"
                           "    \"date\": {},\n"
                           "    \"sender\": \"{}\",\n"
                           "    \"text\": \"{}\"\n"
                           "  }}{}",
                           m.id, m.chat_id, m.date, escape_json_field(m.sender),
                           escape_json_field(m.text),
                           (i + 1 < messages.size() ? ",\n" : "\n"));
  }
  outfile << "]\n";
  return {};
}

std::expected<void, std::string>
Exporter::to_csv(const std::vector<MessageRecord> &messages,
                 const std::filesystem::path &out_path) {
  std::ofstream outfile(out_path, std::ios::trunc);
  if (!outfile.is_open()) {
    return std::unexpected("Failed to open file for CSV export: " +
                           out_path.string());
  }

  outfile << "id,chat_id,date,sender,text\n";
  for (const auto &m : messages) {
    outfile << std::format("{},{},{},{},{}\n", m.id, m.chat_id, m.date,
                           escape_csv_field(m.sender),
                           escape_csv_field(m.text));
  }
  return {};
}

} // namespace grm
