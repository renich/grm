#pragma once

#include <expected>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

struct json_object;

namespace grm {

class JsonValue {
public:
  JsonValue();
  explicit JsonValue(json_object *raw_obj, bool take_ownership = true);
  ~JsonValue();

  JsonValue(const JsonValue &other);
  JsonValue &operator=(const JsonValue &other);

  JsonValue(JsonValue &&other) noexcept;
  JsonValue &operator=(JsonValue &&other) noexcept;

  [[nodiscard]] static std::expected<JsonValue, std::string>
  parse(std::string_view json_str);

  [[nodiscard]] bool is_null() const noexcept;
  [[nodiscard]] bool is_object() const noexcept;
  [[nodiscard]] bool is_array() const noexcept;

  [[nodiscard]] std::optional<std::string> get_type() const;
  [[nodiscard]] std::optional<std::string>
  get_string(const std::string &key) const;
  [[nodiscard]] std::optional<int64_t> get_int(const std::string &key) const;
  [[nodiscard]] std::optional<bool> get_bool(const std::string &key) const;

  [[nodiscard]] std::optional<int64_t> as_int64() const;
  [[nodiscard]] std::optional<std::string> as_string() const;

  [[nodiscard]] std::optional<JsonValue>
  get_object(const std::string &key) const;

  [[nodiscard]] std::vector<JsonValue> get_array(const std::string &key) const;
  [[nodiscard]] std::vector<JsonValue> as_array() const;

  [[nodiscard]] std::string to_string() const;
  [[nodiscard]] json_object *raw() const noexcept { return obj_; }

private:
  json_object *obj_{nullptr};
};

[[nodiscard]] std::string escape_json_string(std::string_view input);

} // namespace grm
