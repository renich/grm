#include "grm/app.hpp"
#include "grm/downloader.hpp"
#include "grm/json_utils.hpp"
#include "grm/logger.hpp"
#include <charconv>
#include <filesystem>
#include <format>
#include <iostream>

namespace grm {

static std::expected<int64_t, std::string> parse_int64(std::string_view str) {
  int64_t val = 0;
  auto [ptr, ec] = std::from_chars(str.data(), str.data() + str.size(), val);
  if (ec != std::errc{} || ptr != str.data() + str.size()) {
    return std::unexpected("Invalid integer: " + std::string(str));
  }
  return val;
}

std::expected<int, std::string>
App::cmd_file_get(const std::vector<std::string> &args) {
  if (args.size() < 2) {
    return std::unexpected(
        "Usage: grm file get [-o|--output <dir|file>] [-t|--topic <id>] <chat_id> <message_ids...>");
  }

  int64_t chat_id = 0;
  bool chat_set = false;
  std::filesystem::path out_dir = ".";
  int64_t topic_id = 0;
  std::vector<int64_t> message_ids;

  for (size_t i = 0; i < args.size(); ++i) {
    std::string_view arg(args[i]);
    if ((arg == "-o" || arg == "--output") && i + 1 < args.size()) {
      out_dir = args[++i];
    } else if ((arg == "-t" || arg == "--topic") && i + 1 < args.size()) {
      if (auto tid = parse_int64(args[++i])) topic_id = *tid;
    } else if (!chat_set && parse_int64(arg).has_value()) {
      chat_id = *parse_int64(arg);
      chat_set = true;
    } else if (chat_set && parse_int64(arg).has_value()) {
      message_ids.push_back(*parse_int64(arg));
    }
  }

  if (!chat_set || message_ids.empty()) {
    return std::unexpected(
        "Usage: grm file get [-o|--output <dir|file>] [-t|--topic <id>] <chat_id> <message_ids...>");
  }

  if (auto res = ensure_authenticated(); !res) return std::unexpected(res.error());
  ensure_chat_loaded(chat_id);

  for (int64_t msg_id : message_ids) {
    const std::string get_msg_req = std::format(
        R"({{"chat_id": {}, "message_id": {}}})", chat_id, msg_id);
    auto msg_res = client_->send_request("getMessage", get_msg_req, 5.0);
    if (!msg_res) {
      grm::log::warn("Failed to get message " + std::to_string(msg_id) + ": " + msg_res.error());
      continue;
    }

    int32_t file_id = 0;
    if (auto content = msg_res->get_object("content")) {
      if (auto doc = content->get_object("document")) {
        if (auto f = doc->get_object("document")) {
          file_id = static_cast<int32_t>(f->get_int("id").value_or(0));
        }
      } else if (auto photo = content->get_object("photo")) {
        auto sizes = photo->get_array("sizes");
        if (!sizes.empty()) {
          if (auto f = sizes.back().get_object("photo")) {
            file_id = static_cast<int32_t>(f->get_int("id").value_or(0));
          }
        }
      }
    }

    if (file_id <= 0) {
      grm::log::warn("No downloadable attachment found in message " + std::to_string(msg_id));
      continue;
    }

    auto dl_payload = Downloader::build_download_file_payload(file_id, 3);
    if (!dl_payload) continue;

    grm::log::info(std::format("Downloading file_id {} from message {}...", file_id, msg_id));
    auto dl_res = client_->send_request("downloadFile", *dl_payload, 60.0);
    if (!dl_res) {
      grm::log::error("Failed to download file: " + dl_res.error());
    } else {
      grm::log::info("File downloaded successfully.");
    }
  }

  return 0;
}

std::expected<int, std::string>
App::cmd_file_download_all(const std::vector<std::string> &args) {
  if (args.empty()) {
    return std::unexpected(
        "Usage: grm file download-all [-o|--output <dir>] [-t|--topic <id>] [-n|--limit <N>] [--type photo|video|doc|audio|all] <chat_id>");
  }

  int64_t chat_id = 0;
  bool chat_set = false;
  std::filesystem::path out_dir = "./downloads";
  int64_t topic_id = 0;
  int limit = 100;
  std::string requested_type = "all";

  for (size_t i = 0; i < args.size(); ++i) {
    std::string_view arg(args[i]);
    if ((arg == "-o" || arg == "--output") && i + 1 < args.size()) {
      out_dir = args[++i];
    } else if ((arg == "-t" || arg == "--topic") && i + 1 < args.size()) {
      if (auto tid = parse_int64(args[++i])) topic_id = *tid;
    } else if ((arg == "-n" || arg == "--limit") && i + 1 < args.size()) {
      if (auto lim = parse_int64(args[++i])) limit = static_cast<int>(*lim);
    } else if (arg == "--type" && i + 1 < args.size()) {
      requested_type = args[++i];
    } else if (!chat_set && parse_int64(arg).has_value()) {
      chat_id = *parse_int64(arg);
      chat_set = true;
    }
  }

  if (!chat_set) {
    return std::unexpected(
        "Usage: grm file download-all [-o|--output <dir>] [-t|--topic <id>] [-n|--limit <N>] [--type photo|video|doc|audio|all] <chat_id>");
  }

  if (auto res = ensure_authenticated(); !res) return std::unexpected(res.error());
  ensure_chat_loaded(chat_id);

  grm::log::info(std::format("Scanning chat {} (topic {}) for media attachments...", chat_id, topic_id));

  std::string method = (topic_id > 0) ? "getForumTopicHistory" : "getChatHistory";
  std::string req_payload;
  if (topic_id > 0) {
    req_payload = std::format(
        R"({{
          "chat_id": {},
          "forum_topic_id": {},
          "message_thread_id": {},
          "from_message_id": 0,
          "offset": 0,
          "limit": {}
        }})",
        chat_id, topic_id, topic_id, std::min(100, limit));
  } else {
    req_payload = std::format(
        R"({{
          "chat_id": {},
          "from_message_id": 0,
          "offset": 0,
          "limit": {},
          "only_local": false
        }})",
        chat_id, std::min(100, limit));
  }

  auto history_res = client_->send_request(method, req_payload, 15.0);
  if (!history_res) {
    return std::unexpected("Failed to scan chat history for downloads: " + history_res.error());
  }

  auto msgs = history_res->get_array("messages");
  int downloaded_count = 0;

  for (const auto &m : msgs) {
    auto content = m.get_object("content");
    if (!content) continue;

    std::string type_str = content->get_type().value_or("");
    if (!Downloader::matches_file_type(type_str, requested_type)) continue;

    int32_t file_id = 0;
    if (type_str == "messageDocument") {
      if (auto doc = content->get_object("document")) {
        if (auto f = doc->get_object("document")) {
          file_id = static_cast<int32_t>(f->get_int("id").value_or(0));
        }
      }
    } else if (type_str == "messagePhoto") {
      auto sizes = content->get_array("sizes");
      if (!sizes.empty()) {
        if (auto f = sizes.back().get_object("photo")) {
          file_id = static_cast<int32_t>(f->get_int("id").value_or(0));
        }
      }
    }

    if (file_id > 0) {
      auto dl_payload = Downloader::build_download_file_payload(file_id, 1);
      if (dl_payload) {
        (void)client_->send_request("downloadFile", *dl_payload, 30.0);
        downloaded_count++;
      }
    }
  }

  grm::log::info(std::format("Initiated bulk download for {} attachment(s) to {}", downloaded_count, out_dir.string()));
  return 0;
}

} // namespace grm
