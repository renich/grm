#pragma once

#include "grm/config.hpp"
#include "grm/logger.hpp"
#include "grm/td_client.hpp"
#include <condition_variable>
#include <expected>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

namespace grm {

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
  static bool is_help_requested(const std::vector<std::string> &args);

private:
  [[nodiscard]] std::expected<int, std::string> cmd_login();
  [[nodiscard]] std::expected<int, std::string>
  cmd_chat_ls(const std::vector<std::string> &args);
  [[nodiscard]] std::expected<int, std::string>
  cmd_msg_ls(const std::vector<std::string> &args);
  [[nodiscard]] std::expected<int, std::string>
  cmd_msg_export(const std::vector<std::string> &args);
  [[nodiscard]] std::expected<int, std::string>
  cmd_msg_search(const std::vector<std::string> &args);
  [[nodiscard]] std::expected<int, std::string>
  cmd_msg_send(const std::vector<std::string> &args);
  [[nodiscard]] std::expected<int, std::string>
  cmd_topic_ls(const std::vector<std::string> &args);


  [[nodiscard]] std::expected<void, std::string> ensure_authenticated();
  [[nodiscard]] std::expected<void, std::string> init_tdlib();
  void ensure_chat_loaded(int64_t chat_id);

  [[nodiscard]] std::string get_auth_state() const;

  void update_auth_state(std::string state, bool closed = false);

  Config config_;
  CliOptions options_;
  std::unique_ptr<TdClient> client_;
  mutable std::mutex auth_mutex_;
  std::condition_variable auth_cv_;
  std::string auth_state_;
  bool is_closed_{false};
};

} // namespace grm
