#include "grm/app.hpp"
#include "grm/json_utils.hpp"
#include "grm/logger.hpp"
#include <chrono>
#include <format>
#include <iostream>
#include <termios.h>
#include <thread>
#include <unistd.h>

namespace grm {

namespace {

std::string read_secure_password(std::string_view prompt) {
  std::cout << prompt;
  std::cout.flush();

  termios oldt{};
  tcgetattr(STDIN_FILENO, &oldt);
  termios newt = oldt;
  newt.c_lflag &= static_cast<tcflag_t>(~ECHO);
  tcsetattr(STDIN_FILENO, TCSANOW, &newt);

  std::string password;
  std::cin >> password;

  tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
  std::cout << '\n';

  return password;
}

} // namespace

std::expected<int, std::string> App::cmd_login() {
  if (config_.api_id == 0 || config_.api_hash.empty()) {
    return std::unexpected(
        "Missing Telegram API credentials. Please set 'api_id' and 'api_hash' in "
        "~/.config/grm/config.json (or pass via -c/--config).");
  }

  if (auto res = init_tdlib(); !res) {
    return std::unexpected(res.error());
  }

  grm::log::auth("Initializing authentication flow...");

  std::string last_state;

  while (true) {
    const std::string state = get_auth_state();

    if (state == "authorizationStateReady") {
      grm::log::auth("Authenticated successfully as user.");
      return 0;
    }

    if (state == "authorizationStateClosed") {
      return std::unexpected("TDLib session closed");
    }

    if (!state.empty() && state != last_state) {
      last_state = state;
      grm::log::verbose("Authorization state changed: " + state);

      if (state == "authorizationStateWaitTdlibParameters") {
        grm::log::info("Configuring TDLib engine parameters...");
        send_tdlib_parameters();
      } else if (state == "authorizationStateWaitPhoneNumber") {
        std::string phone = options_.phone;
        if (phone.empty()) {
          std::cout << "[AUTH] Enter your Telegram phone number (e.g. "
                       "+521234567890): "
                    << std::flush;
          std::cin >> phone;
        } else {
          grm::log::auth("Using pre-filled phone number: " + phone);
        }

        const std::string payload = std::format(
            R"({{
              "phone_number": "{}"
            }})",
            escape_json_string(phone));

        grm::log::auth("Submitting phone number to Telegram...");
        auto res = client_->send_request("setAuthenticationPhoneNumber",
                                         payload, 15.0);
        if (!res) {
          grm::log::error("Failed to set phone number: " + res.error());
          options_.phone.clear(); // Clear invalid option
          last_state.clear();
        } else {
          grm::log::auth(
              "Phone number submitted. Waiting for authentication code...");
        }

      } else if (state == "authorizationStateWaitCode") {
        std::string code = options_.code;
        if (code.empty()) {
          std::cout << "[AUTH] Enter authentication code (or type 'resend' for "
                       "SMS): "
                    << std::flush;
          std::cin >> code;
        } else {
          grm::log::auth("Using pre-filled code.");
        }

        if (code == "resend") {
          grm::log::auth("Requesting Telegram to resend code via SMS...");
          auto res =
              client_->send_request("resendAuthenticationCode", "{}", 15.0);
          if (!res) {
            grm::log::error("Failed to resend code: " + res.error());
          } else {
            grm::log::auth(
                "SMS resend requested. Please check your mobile phone.");
          }
          last_state.clear();
          continue;
        }

        const std::string payload =
            std::format(R"({{"code": "{}"}})", escape_json_string(code));
        auto res =
            client_->send_request("checkAuthenticationCode", payload, 15.0);
        if (!res) {
          grm::log::error("Invalid code: " + res.error());
          options_.code.clear();
          last_state.clear();
        }
      } else if (state == "authorizationStateWaitPassword") {
        const std::string password =
            read_secure_password("[AUTH] Enter your cloud password: ");

        const std::string payload = std::format(R"({{"password": "{}"}})",
                                                escape_json_string(password));
        auto res =
            client_->send_request("checkAuthenticationPassword", payload, 15.0);
        if (!res) {
          grm::log::error("Invalid cloud password: " + res.error());
          last_state.clear();
        }
      }
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(200));
  }
}

} // namespace grm
