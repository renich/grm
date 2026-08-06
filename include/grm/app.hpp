#pragma once

#include "grm/config.hpp"
#include "grm/logger.hpp"
#include "grm/td_client.hpp"
#include <condition_variable>
#include <expected>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

namespace grm {

struct SenderInfo {
  std::string chosen_name;
  std::string full_name;
  std::string username;
  int64_t id{0};
};

class App {
public:
  explicit App(Config config, CliOptions options = {});
  ~App() = default;

  App(const App &) = delete;
  App &operator=(const App &) = delete;
  App(App &&) = default;
  App &operator=(App &&) = default;

  [[nodiscard]] std::expected<int, std::string>
  run(const std::vector<std::string> &args);

  static void print_usage();
  static void print_version();
  static void print_login_help();
  static void print_chat_help();
  static void print_msg_help();
  static void print_topic_help();
  static void print_file_help();
  static bool is_help_requested(const std::vector<std::string> &args);
  [[nodiscard]] static std::expected<int64_t, std::string>
  parse_since_timestamp(std::string_view raw_str);

private:
  [[nodiscard]] std::expected<int, std::string> cmd_login();
  [[nodiscard]] std::expected<int, std::string>
  cmd_completion(const std::vector<std::string> &args);

  // Chat & Group CRUD
  [[nodiscard]] std::expected<int, std::string>
  cmd_chat_ls(const std::vector<std::string> &args);
  [[nodiscard]] std::expected<int, std::string>
  cmd_chat_create(const std::vector<std::string> &args);
  [[nodiscard]] std::expected<int, std::string>
  cmd_chat_info(const std::vector<std::string> &args);
  [[nodiscard]] std::expected<int, std::string>
  cmd_chat_set_title(const std::vector<std::string> &args);
  [[nodiscard]] std::expected<int, std::string>
  cmd_chat_set_desc(const std::vector<std::string> &args);
  [[nodiscard]] std::expected<int, std::string>
  cmd_chat_pin(const std::vector<std::string> &args);
  [[nodiscard]] std::expected<int, std::string>
  cmd_chat_unpin(const std::vector<std::string> &args);
  [[nodiscard]] std::expected<int, std::string>
  cmd_chat_delete(const std::vector<std::string> &args);

  // Message CRUD
  [[nodiscard]] std::expected<int, std::string>
  cmd_msg_ls(const std::vector<std::string> &args);
  [[nodiscard]] std::expected<int, std::string>
  cmd_msg_export(const std::vector<std::string> &args);
  [[nodiscard]] std::expected<int, std::string>
  cmd_msg_search(const std::vector<std::string> &args);
  [[nodiscard]] std::expected<int, std::string>
  cmd_msg_send(const std::vector<std::string> &args);
  [[nodiscard]] std::expected<int, std::string>
  cmd_msg_info(const std::vector<std::string> &args);
  [[nodiscard]] std::expected<int, std::string>
  cmd_msg_edit(const std::vector<std::string> &args);
  [[nodiscard]] std::expected<int, std::string>
  cmd_msg_pin(const std::vector<std::string> &args);
  [[nodiscard]] std::expected<int, std::string>
  cmd_msg_unpin(const std::vector<std::string> &args);
  [[nodiscard]] std::expected<int, std::string>
  cmd_msg_unpin_all(const std::vector<std::string> &args);
  [[nodiscard]] std::expected<int, std::string>
  cmd_msg_delete(const std::vector<std::string> &args);

  // Supergroup Topic CRUD
  [[nodiscard]] std::expected<int, std::string>
  cmd_topic_ls(const std::vector<std::string> &args);
  [[nodiscard]] std::expected<int, std::string>
  cmd_topic_create(const std::vector<std::string> &args);
  [[nodiscard]] std::expected<int, std::string>
  cmd_topic_info(const std::vector<std::string> &args);
  [[nodiscard]] std::expected<int, std::string>
  cmd_topic_edit(const std::vector<std::string> &args);
  [[nodiscard]] std::expected<int, std::string>
  cmd_topic_toggle_close(const std::vector<std::string> &args, bool close);
  [[nodiscard]] std::expected<int, std::string>
  cmd_topic_toggle_pin(const std::vector<std::string> &args, bool pin);
  [[nodiscard]] std::expected<int, std::string>
  cmd_topic_delete(const std::vector<std::string> &args);

  // File Download Engine
  [[nodiscard]] std::expected<int, std::string>
  cmd_file_get(const std::vector<std::string> &args);
  [[nodiscard]] std::expected<int, std::string>
  cmd_file_download_all(const std::vector<std::string> &args);

  [[nodiscard]] std::expected<void, std::string> ensure_authenticated();
  [[nodiscard]] std::expected<void, std::string> init_tdlib();
  void send_tdlib_parameters();
  void ensure_chat_loaded(int64_t chat_id);
  [[nodiscard]] std::expected<JsonValue, std::string>
  parse_formatted_text(const std::string &text,
                       const std::string &mode = "markdown");

  [[nodiscard]] std::string get_auth_state() const;

  void update_auth_state(std::string state, bool closed = false);

  [[nodiscard]] SenderInfo resolve_sender_info(const JsonValue &message_obj);
  [[nodiscard]] std::string resolve_sender_name(const JsonValue &message_obj);

  Config config_;
  CliOptions options_;
  std::unique_ptr<TdClient> client_;
  mutable std::mutex auth_mutex_;
  std::condition_variable auth_cv_;
  std::string auth_state_;
  bool is_closed_{false};
  std::unordered_map<int64_t, SenderInfo> sender_cache_;
};

} // namespace grm
