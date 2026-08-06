#include "grm/app.hpp"
#include "grm/downloader.hpp"
#include "grm/json_utils.hpp"
#include "grm/logger.hpp"
#include <charconv>
#include <filesystem>
#include <format>
#include <iostream>

namespace grm {

CommandSpec get_file_spec() {
  return CommandSpec{
      "file",
      "Download attachments and media files",
      {
          SubcommandSpec{"get", "[-a|-A|--all] [-o|--output <dir|file>] [-t|--topic <id>] [-n|--limit <N>] [--type photo|video|doc|audio|all] <chat_id> [<message_ids...>]", "Download media and file attachments from chats and topics", {
              OptionSpec{"-a", "--all", "", "Bulk download all attachment files in chat or topic", {}},
              OptionSpec{"-A", "", "", "Bulk download all attachment files in chat or topic (alias for -a)", {}},
              OptionSpec{"-o", "--output", "<dir|file>", "Destination output directory or filepath", {}},
              OptionSpec{"-t", "--topic", "<id>", "Target specific forum topic thread ID", {}},
              OptionSpec{"-n", "--limit", "<N>", "Maximum messages to fetch or search", {}},
              OptionSpec{"", "--type", "<type>", "Filter media type for bulk download (photo, video, doc, audio, all)", {"photo", "video", "doc", "audio", "all"}},
              OptionSpec{"-h", "--help", "", "Show file get help message", {}}
          }}
      },
      {}
  };
}

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
  if (args.empty()) {
    return std::unexpected(
        "Usage: grm file get [-A|--all] [-o|--output <dir|file>] [-t|--topic "
        "<id>] [-n|--limit <N>] [--type photo|video|doc|audio|all] <chat_id> "
        "[<message_ids...>]");
  }

  int64_t chat_id = 0;
  bool chat_set = false;
  bool download_all = false;
  std::filesystem::path out_dir = ".";
  int64_t topic_id = 0;
  int limit = 100;
  std::string requested_type = "all";
  std::vector<int64_t> message_ids;

  for (size_t i = 0; i < args.size(); ++i) {
    std::string_view arg(args[i]);
    if (arg == "-A" || arg == "--all") {
      download_all = true;
    } else if ((arg == "-o" || arg == "--output") && i + 1 < args.size()) {
      out_dir = args[++i];
    } else if ((arg == "-t" || arg == "--topic") && i + 1 < args.size()) {
      if (auto tid = parse_int64(args[++i]))
        topic_id = *tid;
    } else if ((arg == "-n" || arg == "--limit") && i + 1 < args.size()) {
      if (auto lim = parse_int64(args[++i]))
        limit = static_cast<int>(*lim);
    } else if (arg == "--type" && i + 1 < args.size()) {
      requested_type = args[++i];
    } else if (!chat_set && parse_int64(arg).has_value()) {
      chat_id = *parse_int64(arg);
      chat_set = true;
    } else if (chat_set && parse_int64(arg).has_value()) {
      message_ids.push_back(*parse_int64(arg));
    }
  }

  if (!chat_set) {
    return std::unexpected(
        "Usage: grm file get [-A|--all] [-o|--output <dir|file>] [-t|--topic "
        "<id>] [-n|--limit <N>] [--type photo|video|doc|audio|all] <chat_id> "
        "[<message_ids...>]");
  }

  if (auto res = ensure_authenticated(); !res)
    return std::unexpected(res.error());
  ensure_chat_loaded(chat_id);

  if (topic_id > 0) {
    const std::string topic_req = std::format(
        R"({{"chat_id": {}, "message_thread_id": {}}})", chat_id, topic_id);
    auto unused_topic = client_->send_request("getForumTopic", topic_req, 2.0);
    (void)unused_topic;
  }

  if (download_all || message_ids.empty()) {
    grm::log::info(
        std::format("Scanning chat {} (topic {}) for media attachments...",
                    chat_id, topic_id));

    std::string method =
        (topic_id > 0) ? "getForumTopicHistory" : "getChatHistory";
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
            "limit": {}
          }})",
          chat_id, std::min(100, limit));
    }

    auto hist_res = client_->send_request(method, req_payload, 5.0);
    if (!hist_res) {
      return std::unexpected("Failed to fetch history for attachments: " +
                             hist_res.error());
    }

    int downloaded_count = 0;
    auto msgs = hist_res->get_array("messages");
    std::filesystem::create_directories(out_dir);

    for (const auto &msg : msgs) {
      int32_t file_id = 0;
      std::string f_type = "file";
      if (auto content = msg.get_object("content")) {
        std::string c_type =
            content->get_string("@type").value_or("messageText");
        if (c_type == "messagePhoto") {
          f_type = "photo";
          auto sizes = content->get_array("sizes");
          if (!sizes.empty()) {
            if (auto photo = sizes.back().get_object("photo")) {
              file_id = static_cast<int32_t>(photo->get_int("id").value_or(0));
            }
          }
        } else if (c_type == "messageVideo") {
          f_type = "video";
          if (auto video = content->get_object("video")) {
            if (auto vf = video->get_object("video")) {
              file_id = static_cast<int32_t>(vf->get_int("id").value_or(0));
            }
          }
        } else if (c_type == "messageDocument") {
          f_type = "doc";
          if (auto doc = content->get_object("document")) {
            if (auto df = doc->get_object("document")) {
              file_id = static_cast<int32_t>(df->get_int("id").value_or(0));
            }
          }
        } else if (c_type == "messageAudio") {
          f_type = "audio";
          if (auto audio = content->get_object("audio")) {
            if (auto af = audio->get_object("audio")) {
              file_id = static_cast<int32_t>(af->get_int("id").value_or(0));
            }
          }
        }
      }

      if (file_id > 0 &&
          (requested_type == "all" || requested_type == f_type)) {
        auto dl_payload = Downloader::build_download_file_payload(file_id, 3);
        if (dl_payload) {
          client_->send_async("downloadFile", *dl_payload);
          downloaded_count++;
        }
      }
    }

    grm::log::info(
        std::format("Initiated bulk download for {} attachment(s) to {}",
                    downloaded_count, out_dir.string()));
    return 0;
  }

  // Specific message IDs download
  for (int64_t msg_id : message_ids) {
    const std::string get_msg_req =
        std::format(R"({{"chat_id": {}, "message_id": {}}})", chat_id, msg_id);
    auto msg_res = client_->send_request("getMessage", get_msg_req, 5.0);
    if (!msg_res) {
      grm::log::warn("Failed to get message " + std::to_string(msg_id) + ": " +
                     msg_res.error());
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
      grm::log::warn("No downloadable attachment found in message " +
                     std::to_string(msg_id));
      continue;
    }

    auto dl_payload = Downloader::build_download_file_payload(file_id, 3);
    if (!dl_payload)
      continue;

    grm::log::info(std::format("Downloading file_id {} from message {}...",
                               file_id, msg_id));
    auto dl_res = client_->send_request("downloadFile", *dl_payload, 60.0);
    if (!dl_res) {
      grm::log::error("Failed to download file: " + dl_res.error());
    } else {
      grm::log::info("File downloaded successfully.");
    }
  }

  return 0;
}

} // namespace grm
