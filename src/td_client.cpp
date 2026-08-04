#include "grm/td_client.hpp"
#include <chrono>
#include <iostream>
#include <json-c/json.h>
#include <td/telegram/td_json_client.h>

namespace grm {

TdClient::TdClient(Config config) : config_(std::move(config)) {}

TdClient::~TdClient() { stop(); }

std::expected<void, std::string> TdClient::start() {
  if (is_running_) {
    return {};
  }

  client_id_ = td_create_client_id();
  if (client_id_ < 0) {
    return std::unexpected("Failed to create TDLib client instance");
  }

  is_running_ = true;
  receiver_thread_ = std::thread(&TdClient::receiver_loop, this);

  return {};
}

void TdClient::stop() {
  if (!is_running_) {
    return;
  }

  is_running_ = false;
  if (receiver_thread_.joinable()) {
    receiver_thread_.join();
  }

  client_id_ = -1;
}

std::string TdClient::generate_extra_id() {
  const uint64_t count = ++request_counter_;
  return "req_" + std::to_string(count);
}

void TdClient::send_async(const std::string &type,
                          std::string_view payload_json) {
  auto parsed = JsonValue::parse(payload_json);
  json_object *raw_obj = nullptr;

  if (parsed && parsed->is_object()) {
    raw_obj = json_object_get(parsed->raw());
  } else {
    raw_obj = json_object_new_object();
  }

  json_object_object_add(raw_obj, "@type",
                         json_object_new_string(type.c_str()));

  const char *str =
      json_object_to_json_string_ext(raw_obj, JSON_C_TO_STRING_PLAIN);
  td_send(client_id_, str);
  json_object_put(raw_obj);
}

std::expected<JsonValue, std::string>
TdClient::send_request(const std::string &type, std::string_view payload_json,
                       double timeout_seconds) {
  if (!is_running_ || client_id_ < 0) {
    return std::unexpected("TDLib client is not running");
  }

  const std::string extra_id = generate_extra_id();
  std::future<JsonValue> fut;

  {
    std::lock_guard<std::mutex> lock(promises_mutex_);
    fut = pending_promises_[extra_id].get_future();
  }

  auto parsed = JsonValue::parse(payload_json);
  json_object *raw_obj = nullptr;

  if (parsed && parsed->is_object()) {
    raw_obj = json_object_get(parsed->raw());
  } else {
    raw_obj = json_object_new_object();
  }

  json_object_object_add(raw_obj, "@type",
                         json_object_new_string(type.c_str()));
  json_object_object_add(raw_obj, "@extra",
                         json_object_new_string(extra_id.c_str()));

  const char *str =
      json_object_to_json_string_ext(raw_obj, JSON_C_TO_STRING_PLAIN);
  td_send(client_id_, str);
  json_object_put(raw_obj);

  const auto status =
      fut.wait_for(std::chrono::duration<double>(timeout_seconds));

  if (status == std::future_status::timeout) {
    std::lock_guard<std::mutex> lock(promises_mutex_);
    pending_promises_.erase(extra_id);
    return std::unexpected("Request timed out (" + type + ")");
  }

  return fut.get();
}

void TdClient::on_update(UpdateCallback callback) {
  std::lock_guard<std::mutex> lock(callback_mutex_);
  callbacks_.push_back(std::move(callback));
}

void TdClient::receiver_loop() {
  while (is_running_) {
    const char *raw_res = td_receive(1.0);
    if (!raw_res) {
      continue;
    }

    auto parsed = JsonValue::parse(raw_res);
    if (parsed) {
      handle_incoming(*parsed);
    }
  }
}

void TdClient::handle_incoming(const JsonValue &value) {
  if (auto extra = value.get_string("@extra")) {
    std::promise<JsonValue> prom;
    bool found = false;

    {
      std::lock_guard<std::mutex> lock(promises_mutex_);
      auto it = pending_promises_.find(*extra);
      if (it != pending_promises_.end()) {
        prom = std::move(it->second);
        pending_promises_.erase(it);
        found = true;
      }
    }

    if (found) {
      prom.set_value(value);
      return;
    }
  }

  // Dispatch to subscriber callbacks
  std::vector<UpdateCallback> callbacks_copy;
  {
    std::lock_guard<std::mutex> lock(callback_mutex_);
    callbacks_copy = callbacks_;
  }

  for (const auto &cb : callbacks_copy) {
    cb(value);
  }
}

} // namespace grm
