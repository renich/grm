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

  [[nodiscard]] std::expected<int, std::string>
  run(const std::vector<std::string> &args);

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
  cmd_extract_bday(const std::vector<std::string> &args);
  [[nodiscard]] std::expected<int, std::string>
  cmd_send(const std::vector<std::string> &args);
  [[nodiscard]] std::expected<int, std::string>
  cmd_send_file(const std::vector<std::string> &args);
  [[nodiscard]] std::expected<int, std::string>
  cmd_topic_ls(const std::vector<std::string> &args);

  void print_usage();
  void print_version();
  [[nodiscard]] std::expected<void, std::string> ensure_authenticated();
  [[nodiscard]] std::expected<void, std::string> init_tdlib();

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
