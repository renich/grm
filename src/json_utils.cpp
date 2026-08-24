#include "grm/json_utils.hpp"
#include <charconv>
#include <cstring>
#include <format>
#include <json-c/json.h>
#include <mutex>

namespace grm {

namespace {
std::mutex g_json_c_mutex;
} // namespace

json_object *json_c_get(json_object *obj) {
  if (!obj)
    return nullptr;
  std::lock_guard<std::mutex> lock(g_json_c_mutex);
  return json_object_get(obj);
}

void json_c_put(json_object *obj) {
  if (!obj)
    return;
  std::lock_guard<std::mutex> lock(g_json_c_mutex);
  json_object_put(obj);
}

JsonValue::JsonValue() = default;

JsonValue::JsonValue(json_object *raw_obj, bool take_ownership)
    : obj_(raw_obj) {
  if (obj_ && !take_ownership) {
    json_c_get(obj_);
  }
}

JsonValue::~JsonValue() {
  if (obj_) {
    json_c_put(obj_);
    obj_ = nullptr;
  }
}

JsonValue::JsonValue(const JsonValue &other) {
  if (other.obj_) {
    obj_ = json_c_get(other.obj_);
  } else {
    obj_ = nullptr;
  }
}

JsonValue &JsonValue::operator=(const JsonValue &other) {
  if (this != &other) {
    if (obj_) {
      json_c_put(obj_);
    }
    if (other.obj_) {
      obj_ = json_c_get(other.obj_);
    } else {
      obj_ = nullptr;
    }
  }
  return *this;
}

JsonValue::JsonValue(JsonValue &&other) noexcept : obj_(other.obj_) {
  other.obj_ = nullptr;
}

JsonValue &JsonValue::operator=(JsonValue &&other) noexcept {
  if (this != &other) {
    if (obj_) {
      json_c_put(obj_);
    }
    obj_ = other.obj_;
    other.obj_ = nullptr;
  }
  return *this;
}

std::expected<JsonValue, std::string>
JsonValue::parse(std::string_view json_str) {
  json_tokener *tok = json_tokener_new();
  if (!tok) {
    return std::unexpected("Failed to allocate JSON tokener");
  }

  const auto len = static_cast<int>(json_str.size());
  // NOLINTNEXTLINE(bugprone-suspicious-stringview-data-usage)
  json_object *parsed = json_tokener_parse_ex(tok, json_str.data(), len);
  const json_tokener_error err = json_tokener_get_error(tok);
  json_tokener_free(tok);

  if (err != json_tokener_success || !parsed) {
    if (parsed) {
      json_c_put(parsed);
    }
    return std::unexpected("JSON parse error: " +
                           std::string(json_tokener_error_desc(err)));
  }

  return JsonValue(parsed, true);
}

bool JsonValue::is_null() const noexcept {
  return !obj_ || json_object_is_type(obj_, json_type_null);
}

bool JsonValue::is_object() const noexcept {
  return obj_ && json_object_is_type(obj_, json_type_object);
}

bool JsonValue::is_array() const noexcept {
  return obj_ && json_object_is_type(obj_, json_type_array);
}

std::optional<std::string> JsonValue::get_type() const {
  return get_string("@type");
}

std::optional<std::string> JsonValue::get_string(const std::string &key) const {
  if (key.empty()) {
    return as_string();
  }
  if (!is_object())
    return std::nullopt;

  json_object *val = nullptr;
  if (json_object_object_get_ex(obj_, key.c_str(), &val) && val) {
    if (json_object_is_type(val, json_type_string)) {
      return std::string(json_object_get_string(val));
    }
    if (json_object_is_type(val, json_type_int)) {
      return std::to_string(json_object_get_int64(val));
    }
  }
  return std::nullopt;
}

std::optional<int64_t> JsonValue::get_int(const std::string &key) const {
  if (key.empty()) {
    return as_int64();
  }
  if (!is_object())
    return std::nullopt;

  json_object *val = nullptr;
  if (json_object_object_get_ex(obj_, key.c_str(), &val) && val) {
    if (json_object_is_type(val, json_type_int)) {
      return json_object_get_int64(val);
    }
    if (json_object_is_type(val, json_type_string)) {
      const char *str = json_object_get_string(val);
      int64_t parsed_val = 0;
      auto [ptr, ec] = std::from_chars(str, str + std::strlen(str), parsed_val);
      if (ec == std::errc{}) {
        return parsed_val;
      }
    }
  }
  return std::nullopt;
}

std::optional<double> JsonValue::as_double() const {
  if (obj_) {
    if (json_object_is_type(obj_, json_type_double)) {
      return json_object_get_double(obj_);
    }
    if (json_object_is_type(obj_, json_type_int)) {
      return static_cast<double>(json_object_get_int64(obj_));
    }
  }
  return std::nullopt;
}

std::optional<double> JsonValue::get_double(const std::string &key) const {
  if (key.empty()) {
    return as_double();
  }
  if (!is_object())
    return std::nullopt;

  json_object *val = nullptr;
  if (json_object_object_get_ex(obj_, key.c_str(), &val) && val) {
    if (json_object_is_type(val, json_type_double)) {
      return json_object_get_double(val);
    }
    if (json_object_is_type(val, json_type_int)) {
      return static_cast<double>(json_object_get_int64(val));
    }
  }
  return std::nullopt;
}

std::optional<bool> JsonValue::get_bool(const std::string &key) const {
  if (key.empty()) {
    if (obj_ && json_object_is_type(obj_, json_type_boolean)) {
      return json_object_get_boolean(obj_) != 0;
    }
    return std::nullopt;
  }
  if (!is_object())
    return std::nullopt;

  json_object *val = nullptr;
  if (json_object_object_get_ex(obj_, key.c_str(), &val) && val) {
    if (json_object_is_type(val, json_type_boolean)) {
      return json_object_get_boolean(val) != 0;
    }
  }
  return std::nullopt;
}

std::optional<int64_t> JsonValue::as_int64() const {
  if (obj_) {
    if (json_object_is_type(obj_, json_type_int)) {
      return json_object_get_int64(obj_);
    }
    if (json_object_is_type(obj_, json_type_string)) {
      const char *str = json_object_get_string(obj_);
      int64_t parsed_val = 0;
      auto [ptr, ec] = std::from_chars(str, str + std::strlen(str), parsed_val);
      if (ec == std::errc{}) {
        return parsed_val;
      }
    }
  }
  return std::nullopt;
}

std::optional<std::string> JsonValue::as_string() const {
  if (obj_) {
    if (json_object_is_type(obj_, json_type_string)) {
      return std::string(json_object_get_string(obj_));
    }
    if (json_object_is_type(obj_, json_type_int)) {
      return std::to_string(json_object_get_int64(obj_));
    }
  }
  return std::nullopt;
}

std::optional<JsonValue> JsonValue::get_object(const std::string &key) const {
  if (!is_object())
    return std::nullopt;

  json_object *val = nullptr;
  if (json_object_object_get_ex(obj_, key.c_str(), &val) && val) {
    if (json_object_is_type(val, json_type_object)) {
      return JsonValue(val, false);
    }
  }
  return std::nullopt;
}

std::vector<JsonValue> JsonValue::get_array(const std::string &key) const {
  std::vector<JsonValue> result;
  if (!is_object())
    return result;

  json_object *arr = nullptr;
  if (json_object_object_get_ex(obj_, key.c_str(), &arr) && arr) {
    if (json_object_is_type(arr, json_type_array)) {
      const size_t len = json_object_array_length(arr);
      result.reserve(len);
      for (size_t i = 0; i < len; ++i) {
        json_object *elem = json_object_array_get_idx(arr, i);
        if (elem) {
          result.emplace_back(elem, false);
        }
      }
    }
  }
  return result;
}

std::vector<JsonValue> JsonValue::as_array() const {
  std::vector<JsonValue> result;
  if (!is_array())
    return result;

  const size_t len = json_object_array_length(obj_);
  result.reserve(len);
  for (size_t i = 0; i < len; ++i) {
    json_object *elem = json_object_array_get_idx(obj_, i);
    if (elem) {
      result.emplace_back(elem, false);
    }
  }
  return result;
}

std::string JsonValue::to_string() const {
  if (!obj_) {
    return "null";
  }
  return json_object_to_json_string_ext(obj_, JSON_C_TO_STRING_PLAIN);
}

std::string escape_json_string(std::string_view input) {
  std::string escaped;
  escaped.reserve(input.size() + 8);
  for (char c : input) {
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
      if (static_cast<unsigned char>(c) < 0x20) {
        escaped += std::format("\\u{:04x}", static_cast<unsigned char>(c));
      } else {
        escaped += c;
      }
      break;
    }
  }
  return escaped;
}

} // namespace grm
