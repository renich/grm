#include "grm/app.hpp"
#include "grm/json_utils.hpp"
#include <chrono>
#include <format>
#include <iostream>
#include <thread>

namespace grm {

std::expected<int, std::string> App::cmd_login() {
  if (auto res = init_tdlib(); !res) {
    return std::unexpected(res.error());
  }

  std::cout << "Initializing login flow..." << std::endl;

  std::string last_state;

  while (true) {
    const std::string state = get_auth_state();

    if (state == "authorizationStateReady") {
      std::cout << "✓ Authenticated successfully as user!" << std::endl;
      return 0;
    }

    if (state == "authorizationStateClosed") {
      return std::unexpected("TDLib session closed");
    }

    if (!state.empty() && state != last_state) {
      last_state = state;

      if (state == "authorizationStateWaitPhoneNumber") {
        std::cout << "Enter your Telegram phone number (e.g. +521234567890): ";
        std::string phone;
        std::cin >> phone;

        const std::string payload = std::format(
            R"({{
              "phone_number": "{}",
              "settings": {{
                "@type": "phoneNumberAuthenticationSettings",
                "allow_flash_call": false,
                "is_current_phone_number": false,
                "allow_sms_retriever_api": false
              }}
            }})",
            escape_json_string(phone));

        std::cout << "Submitting phone number to Telegram..." << std::endl;
        auto res = client_->send_request("setAuthenticationPhoneNumber",
                                         payload, 15.0);
        if (!res) {
          std::cerr << "Failed to set phone number: " << res.error()
                    << std::endl;
          last_state.clear(); // Allow user to re-enter phone
        } else {
          std::cout << "Phone number submitted. Waiting for code..."
                    << std::endl;
        }

      } else if (state == "authorizationStateWaitCode") {
        std::cout << "Enter the authentication code sent by Telegram (or type "
                     "'resend' to request SMS): ";
        std::string code;
        std::cin >> code;

        if (code == "resend") {
          std::cout << "Requesting Telegram to resend code via SMS..."
                    << std::endl;
          auto res =
              client_->send_request("resendAuthenticationCode", "{}", 15.0);
          if (!res) {
            std::cerr << "Failed to resend code: " << res.error() << std::endl;
          } else {
            std::cout << "✓ Resend requested. Check your mobile phone SMS."
                      << std::endl;
          }
          last_state.clear();
          continue;
        }

        const std::string payload =
            std::format(R"({{"code": "{}"}})", escape_json_string(code));
        auto res =
            client_->send_request("checkAuthenticationCode", payload, 15.0);
        if (!res) {
          std::cerr << "Invalid code: " << res.error() << std::endl;
          last_state.clear(); // Allow user to re-enter code
        }
      } else if (state == "authorizationStateWaitPassword") {
        std::cout << "Enter your 2FA password: ";
        std::string password;
        std::cin >> password;

        const std::string payload = std::format(R"({{"password": "{}"}})",
                                                escape_json_string(password));
        auto res =
            client_->send_request("checkAuthenticationPassword", payload, 15.0);
        if (!res) {
          std::cerr << "Invalid password: " << res.error() << std::endl;
          last_state.clear(); // Allow user to re-enter password
        }
      }
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(200));
  }
}

} // namespace grm
