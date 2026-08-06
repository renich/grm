#include "grm/app.hpp"
#include "grm/command_registry.hpp"
#include <algorithm>
#include <chrono>
#include <format>
#include <iostream>
#include <thread>

namespace grm {

App::App(Config config, CliOptions options)
    : config_(std::move(config)), options_(std::move(options)) {}

void App::print_version() { std::cout << "grm 0.5.1\n"; }

void App::print_usage(fmt::OutputFormat format) {
  if (format == fmt::OutputFormat::Json || format == fmt::OutputFormat::JsonL) {
    std::cout << CommandRegistry::get_instance().render_global_help_json();
  } else {
    std::cout << CommandRegistry::get_instance().render_global_help();
  }
}

bool App::is_help_requested(const std::vector<std::string> &args) {
  return std::ranges::any_of(args, [](const std::string &arg) {
    return arg == "-h" || arg == "--help" || arg == "--help=all";
  });
}

void App::print_login_help(fmt::OutputFormat format) {
  if (format == fmt::OutputFormat::Json || format == fmt::OutputFormat::JsonL) {
    std::cout << CommandRegistry::get_instance().render_command_help_json("login");
  } else {
    std::cout << CommandRegistry::get_instance().render_command_help("login");
  }
}

void App::print_chat_help(fmt::OutputFormat format) {
  if (format == fmt::OutputFormat::Json || format == fmt::OutputFormat::JsonL) {
    std::cout << CommandRegistry::get_instance().render_command_help_json("chat");
  } else {
    std::cout << CommandRegistry::get_instance().render_command_help("chat");
  }
}

void App::print_msg_help(fmt::OutputFormat format) {
  if (format == fmt::OutputFormat::Json || format == fmt::OutputFormat::JsonL) {
    std::cout << CommandRegistry::get_instance().render_command_help_json("msg");
  } else {
    std::cout << CommandRegistry::get_instance().render_command_help("msg");
  }
}

void App::print_topic_help(fmt::OutputFormat format) {
  if (format == fmt::OutputFormat::Json || format == fmt::OutputFormat::JsonL) {
    std::cout << CommandRegistry::get_instance().render_command_help_json("topic");
  } else {
    std::cout << CommandRegistry::get_instance().render_command_help("topic");
  }
}

void App::print_file_help(fmt::OutputFormat format) {
  if (format == fmt::OutputFormat::Json || format == fmt::OutputFormat::JsonL) {
    std::cout << CommandRegistry::get_instance().render_command_help_json("file");
  } else {
    std::cout << CommandRegistry::get_instance().render_command_help("file");
  }
}

std::string App::get_auth_state() const {
  std::scoped_lock lock(auth_mutex_);
  return auth_state_;
}

void App::update_auth_state(std::string state, bool closed) {
  {
    std::scoped_lock lock(auth_mutex_);
    auth_state_ = std::move(state);
    if (closed) {
      is_closed_ = true;
    }
  }
  auth_cv_.notify_all();
}

std::expected<void, std::string> App::init_tdlib() {
  if (client_) {
    return {};
  }

  client_ = std::make_unique<TdClient>(config_);

  client_->on_update([this](const JsonValue &update) {
    if (auto type = update.get_type()) {
      if (*type == "updateAuthorizationState") {
        if (auto state = update.get_object("authorization_state")) {
          if (auto sttype = state->get_type()) {
            grm::log::debug("[Auth State]: " + *sttype);
            if (*sttype == "authorizationStateWaitCode") {
              if (auto code_info = state->get_object("code_info")) {
                if (auto code_type_obj = code_info->get_object("type")) {
                  if (auto code_type = code_type_obj->get_type()) {
                    if (*code_type == "authenticationCodeTypeTelegramMessage") {
                      grm::log::auth("Code sent as an in-app message to your "
                                     "active Telegram client (Chat: Telegram "
                                     "Service Notifications).");
                    } else if (*code_type == "authenticationCodeTypeSms") {
                      grm::log::auth("Code sent via SMS to your phone number.");
                    } else if (*code_type == "authenticationCodeTypeCall") {
                      grm::log::auth("Code will be delivered via an automated "
                                     "phone call.");
                    } else {
                      grm::log::auth("Code delivery method: " + *code_type);
                    }
                  }
                }
              }
            }
            update_auth_state(*sttype, *sttype == "authorizationStateClosed");

            if (*sttype == "authorizationStateWaitTdlibParameters") {
              send_tdlib_parameters();
            } else if (*sttype == "authorizationStateWaitEncryptionKey") {
              client_->send_async("checkDatabaseEncryptionKey",
                                  R"({"encryption_key": ""})");
            }
          }
        }
      }
    }
  });

  if (auto res = client_->start(); !res) {
    return std::unexpected(res.error());
  }

  // Trigger initial TDLib authorization state update
  client_->send_async("getAuthorizationState", "{}");

  return {};
}

void App::send_tdlib_parameters() {
  if (!client_)
    return;

  const auto db_path = options_.use_test_dc
                           ? config_.config_dir / "tdlib_test_db"
                           : config_.db_dir;
  if (options_.use_test_dc) {
    grm::log::info("Connecting to Telegram Test Data Center (Test DC)...");
  }

  const std::string params = std::format(
      R"({{
        "@type": "setTdlibParameters",
        "use_test_dc": {},
        "database_directory": "{}",
        "files_directory": "{}/files",
        "database_encryption_key": "",
        "use_file_database": true,
        "use_chat_info_database": true,
        "use_message_database": true,
        "use_secret_chats": true,
        "api_id": {},
        "api_hash": "{}",
        "system_language_code": "en",
        "device_model": "grm",
        "system_version": "Linux x86_64",
        "application_version": "0.5.1"
      }})",
      options_.use_test_dc ? "true" : "false", db_path.string(),
      db_path.string(), config_.api_id, config_.api_hash);

  client_->send_async("setTdlibParameters", params);
}

std::expected<void, std::string> App::ensure_authenticated() {
  if (auto res = init_tdlib(); !res) {
    return std::unexpected(res.error());
  }

  // Block and wait up to 5 seconds for TDLib session database to load and reach authorizationStateReady
  {
    std::unique_lock<std::mutex> lock(auth_mutex_);
    auth_cv_.wait_for(lock, std::chrono::seconds(5), [this] {
      return auth_state_ == "authorizationStateReady" || is_closed_;
    });
  }

  const std::string current_state = get_auth_state();

  if (current_state == "authorizationStateReady") {
    return {};
  }

  return std::unexpected(
      "Not authenticated. Please run 'grm login' to authenticate first.");
}

void App::ensure_chat_loaded(int64_t chat_id) {
  if (!client_) {
    return;
  }
  const std::string chat_req = std::format(R"({{"chat_id": {}}})", chat_id);
  auto chat_res = client_->send_request("getChat", chat_req, 3.0);
  if (!chat_res) {
    auto load_res =
        client_->send_request("loadChats", R"({"limit": 100})", 5.0);
    if (!load_res) {
      grm::log::debug("loadChats: " + load_res.error());
    }
    for (int i = 0; i < 10; ++i) {
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
      if (client_->send_request("getChat", chat_req, 2.0)) {
        break;
      }
    }
  }
  (void)client_->send_request("openChat", chat_req, 2.0);
}

std::expected<int, std::string> App::run(const std::vector<std::string> &args) {
  if (options_.version) {
    print_version();
    return 0;
  }

  if (args.empty()) {
    print_usage(options_.format);
    return 0;
  }

  if (std::ranges::any_of(args, [](const std::string &arg) {
        return arg == "--help=all";
      }) ||
      (args.size() >= 2 && (args[0] == "--help" || args[0] == "help") && args[1] == "all")) {
    if (options_.format == fmt::OutputFormat::Json || options_.format == fmt::OutputFormat::JsonL) {
      std::cout << CommandRegistry::get_instance().render_all_help_json();
    } else {
      std::cout << CommandRegistry::get_instance().render_all_help();
    }
    return 0;
  }

  const std::string &cmd = args[0];

  if (cmd == "-h" || cmd == "--help" || cmd == "help") {
    print_usage(options_.format);
    return 0;
  }

  std::vector<std::string> sub_args(args.begin() + 1, args.end());

  if (cmd == "login") {
    if (is_help_requested(sub_args)) {
      print_login_help(options_.format);
      return 0;
    }
    for (size_t i = 0; i < sub_args.size(); ++i) {
      if ((sub_args[i] == "-p" || sub_args[i] == "--phone") &&
          i + 1 < sub_args.size()) {
        options_.phone = sub_args[++i];
      } else if ((sub_args[i] == "-k" || sub_args[i] == "--code") &&
                 i + 1 < sub_args.size()) {
        options_.code = sub_args[++i];
      }
    }
    return cmd_login();
  }

  if (cmd == "chat") {
    if (sub_args.empty() || is_help_requested(sub_args)) {
      print_chat_help(options_.format);
      return 0;
    }
    const std::string &sub = sub_args[0];
    std::vector<std::string> sub_opts(sub_args.begin() + 1, sub_args.end());
    if (sub == "ls")
      return cmd_chat_ls(sub_opts);
    if (sub == "create")
      return cmd_chat_create(sub_opts);
    if (sub == "info")
      return cmd_chat_info(sub_opts);
    if (sub == "set-title")
      return cmd_chat_set_title(sub_opts);
    if (sub == "set-desc")
      return cmd_chat_set_desc(sub_opts);
    if (sub == "pin")
      return cmd_chat_pin(sub_opts);
    if (sub == "unpin")
      return cmd_chat_unpin(sub_opts);
    if (sub == "delete")
      return cmd_chat_delete(sub_opts);

    print_chat_help(options_.format);
    return std::unexpected("Unknown chat subcommand: " + sub);
  }

  if (cmd == "msg") {
    if (sub_args.empty() || is_help_requested(sub_args)) {
      print_msg_help(options_.format);
      return 0;
    }
    const std::string &sub = sub_args[0];
    std::vector<std::string> sub_opts(sub_args.begin() + 1, sub_args.end());
    if (sub == "ls")
      return cmd_msg_ls(sub_opts);
    if (sub == "export")
      return cmd_msg_export(sub_opts);
    if (sub == "search")
      return cmd_msg_search(sub_opts);
    if (sub == "send")
      return cmd_msg_send(sub_opts);
    if (sub == "info")
      return cmd_msg_info(sub_opts);
    if (sub == "edit")
      return cmd_msg_edit(sub_opts);
    if (sub == "delete")
      return cmd_msg_delete(sub_opts);

    print_msg_help(options_.format);
    return std::unexpected("Unknown msg subcommand: " + sub);
  }

  if (cmd == "topic") {
    if (sub_args.empty() || is_help_requested(sub_args)) {
      print_topic_help(options_.format);
      return 0;
    }
    const std::string &sub = sub_args[0];
    std::vector<std::string> sub_opts(sub_args.begin() + 1, sub_args.end());
    if (sub == "ls")
      return cmd_topic_ls(sub_opts);
    if (sub == "create")
      return cmd_topic_create(sub_opts);
    if (sub == "info")
      return cmd_topic_info(sub_opts);
    if (sub == "edit")
      return cmd_topic_edit(sub_opts);
    if (sub == "close")
      return cmd_topic_toggle_close(sub_opts, true);
    if (sub == "reopen")
      return cmd_topic_toggle_close(sub_opts, false);
    if (sub == "pin")
      return cmd_topic_toggle_pin(sub_opts, true);
    if (sub == "unpin")
      return cmd_topic_toggle_pin(sub_opts, false);
    if (sub == "delete")
      return cmd_topic_delete(sub_opts);

    print_topic_help(options_.format);
    return std::unexpected("Unknown topic subcommand: " + sub);
  }

  if (cmd == "file") {
    if (sub_args.empty() || is_help_requested(sub_args)) {
      print_file_help(options_.format);
      return 0;
    }
    const std::string &sub = sub_args[0];
    std::vector<std::string> sub_opts(sub_args.begin() + 1, sub_args.end());
    if (sub == "get")
      return cmd_file_get(sub_opts);

    print_file_help(options_.format);
    return std::unexpected("Unknown file subcommand: " + sub);
  }

  if (cmd == "completion") {
    return cmd_completion(sub_args);
  }

  print_usage();
  return std::unexpected("Unknown command: " + cmd);
}

std::expected<JsonValue, std::string>
App::parse_formatted_text(
    const std::string &text,
    const std::string &mode) { // NOLINT(bugprone-easily-swappable-parameters)
  if (text.empty()) {
    std::string empty_payload =
        R"({"@type": "formattedText", "text": "", "entities": []})";
    return *JsonValue::parse(empty_payload);
  }

  std::string parse_mode_type = "textParseModeMarkdown";
  if (mode == "html" || mode == "HTML") {
    parse_mode_type = "textParseModeHTML";
  }

  const std::string payload = std::format(
      R"({{
        "@type": "parseTextEntities",
        "text": "{}",
        "parse_mode": {{
          "@type": "{}",
          "version": 2
        }}
      }})",
      escape_json_string(text), parse_mode_type);

  auto res = client_->send_request("parseTextEntities", payload, 5.0);
  if (!res) {
    grm::log::debug("parseTextEntities failed: " + res.error() +
                    ", falling back to plain formattedText");
    const std::string plain_payload = std::format(
        R"({{
          "@type": "formattedText",
          "text": "{}",
          "entities": []
        }})",
        escape_json_string(text));
    return *JsonValue::parse(plain_payload);
  }

  return *res;
}

} // namespace grm
