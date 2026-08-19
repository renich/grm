#include "grm/td_client.hpp"
#include "grm/logger.hpp"
#include <chrono>
#include <format>
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

  int td_verbosity = 0;
  if (log::get_verbosity() >= log::VerbosityLevel::Debug) {
    td_verbosity = 4;
  } else if (log::get_verbosity() == log::VerbosityLevel::Verbose) {
    td_verbosity = 1;
  }

  const std::string set_log_verb = std::format(
      R"({{"@type": "setLogVerbosityLevel", "new_verbosity_level": {}}})",
      td_verbosity);
  td_execute(set_log_verb.c_str());

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

  if (client_id_ >= 0) {
    send_async("close", "{}");
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
                          std::string_view payload_json) const {
  if (client_id_ < 0)
    return;

  json_object *raw_obj = json_tokener_parse(std::string(payload_json).c_str());
  if (!raw_obj || !json_object_is_type(raw_obj, json_type_object)) {
    if (raw_obj) json_c_put(raw_obj);
    raw_obj = json_object_new_object();
  }

  json_object_object_add(raw_obj, "@type",
                         json_object_new_string(type.c_str()));

  const char *str =
      json_object_to_json_string_ext(raw_obj, JSON_C_TO_STRING_PLAIN);
  td_send(client_id_, str);
  json_c_put(raw_obj);
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
    std::scoped_lock lock(promises_mutex_);
    fut = pending_promises_[extra_id].get_future();
  }

  json_object *raw_obj = json_object_new_object();
  json_object_object_add(raw_obj, "@type",
                         json_object_new_string(type.c_str()));
  json_object_object_add(raw_obj, "@extra",
                         json_object_new_string(extra_id.c_str()));

  auto parsed = JsonValue::parse(payload_json);
  if (parsed && parsed->is_object()) {
    json_object_object_foreach(parsed->raw(), key, val) {
      if (std::string_view(key) != "@type" &&
          std::string_view(key) != "@extra") {
        json_object_object_add(raw_obj, key, json_c_get(val));
      }
    }
  }

  const char *str = json_object_to_json_string_ext(
      raw_obj, JSON_C_TO_STRING_PLAIN | JSON_C_TO_STRING_NOSLASHESCAPE);
  grm::log::debug("TD_SEND: " + std::string(str));
  td_send(client_id_, str);

  json_c_put(raw_obj);

  const auto status =
      fut.wait_for(std::chrono::duration<double>(timeout_seconds));

  if (status == std::future_status::timeout) {
    std::scoped_lock lock(promises_mutex_);
    pending_promises_.erase(extra_id);
    return std::unexpected("Request timed out (" + type + ")");
  }

  auto val = fut.get();
  if (auto t = val.get_type(); t && *t == "error") {
    const std::string msg =
        val.get_string("message").value_or("Unknown TDLib error");
    const int64_t code = val.get_int("code").value_or(0);
    return std::unexpected(std::format("TDLib Error [{}]: {}", code, msg));
  }

  return val;
}

void TdClient::on_update(UpdateCallback callback) {
  std::scoped_lock lock(callback_mutex_);
  callbacks_.push_back(std::move(callback));
}

void TdClient::receiver_loop() {
  while (is_running_) {
    const char *raw_res = td_receive(0.5);
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
  if (auto type = value.get_type()) {
    if (*type == "updateChatFolders") {
      std::scoped_lock lock(chat_folders_mutex_);
      cached_chat_folders_ = value.get_array("chat_folders");
    }
  }

  if (auto extra = value.get_string("@extra")) {
    std::promise<JsonValue> prom;
    bool found = false;

    {
      std::scoped_lock lock(promises_mutex_);
      auto it = pending_promises_.find(*extra);
      if (it != pending_promises_.end()) {
        prom = std::move(it->second);
        pending_promises_.erase(it);
        found = true;
      }
    }

    if (found) {
      prom.set_value(value);
    }
    // Early return for @extra responses so timed-out requests don't leak into subscriber update callbacks
    return;
  }

  // Dispatch to subscriber callbacks
  std::vector<UpdateCallback> callbacks_copy;
  {
    std::scoped_lock lock(callback_mutex_);
    callbacks_copy = callbacks_;
  }

  for (const auto &cb : callbacks_copy) {
    cb(value);
  }
}

std::vector<JsonValue> TdClient::get_cached_chat_folders() const {
  std::scoped_lock lock(chat_folders_mutex_);
  return cached_chat_folders_;
}

} // namespace grm

