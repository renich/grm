#pragma once

#include "grm/config.hpp"
#include "grm/json_utils.hpp"
#include <atomic>
#include <condition_variable>
#include <expected>
#include <functional>
#include <future>
#include <mutex>
#include <string>
#include <thread>
#include <unordered_map>

namespace grm {

class TdClient {
public:
  using UpdateCallback = std::function<void(const JsonValue &)>;

  explicit TdClient(Config config);
  ~TdClient();

  TdClient(const TdClient &) = delete;
  TdClient &operator=(const TdClient &) = delete;

  TdClient(TdClient &&) = delete;
  TdClient &operator=(TdClient &&) = delete;

  [[nodiscard]] std::expected<void, std::string> start();
  void stop();

  [[nodiscard]] std::expected<JsonValue, std::string>
  send_request(const std::string &type, std::string_view payload_json = "{}",
               double timeout_seconds = 10.0);

  void send_async(const std::string &type,
                  std::string_view payload_json = "{}") const;

  void on_update(UpdateCallback callback);

  [[nodiscard]] const Config &config() const noexcept { return config_; }
  [[nodiscard]] int client_id() const noexcept { return client_id_; }

private:
  void receiver_loop();
  void handle_incoming(const JsonValue &value);
  [[nodiscard]] std::string generate_extra_id();

  Config config_;
  int client_id_{-1};
  std::atomic<bool> is_running_{false};
  std::thread receiver_thread_;

  std::mutex promises_mutex_;
  std::unordered_map<std::string, std::promise<JsonValue>> pending_promises_;
  std::atomic<uint64_t> request_counter_{0};

  std::mutex callback_mutex_;
  std::vector<UpdateCallback> callbacks_;
};

} // namespace grm
