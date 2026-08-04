#include "grm/app.hpp"
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

  for (int attempt = 0; attempt < 100; ++attempt) {
    if (auth_state_ == "authorizationStateReady") {
      std::cout << "✓ Authenticated successfully as user!" << std::endl;
      return 0;
    }

    if (auth_state_ == "authorizationStateWaitPhoneNumber") {
      std::cout << "Enter your Telegram phone number (e.g. +521234567890): ";
      std::string phone;
      std::cin >> phone;

      const std::string payload =
          std::format(R"({{"phone_number": "{}"}})", phone);
      auto res =
          client_->send_request("setAuthenticationPhoneNumber", payload, 10.0);
      if (!res) {
        std::cerr << "Failed to set phone number: " << res.error() << std::endl;
      }
    } else if (auth_state_ == "authorizationStateWaitCode") {
      std::cout << "Enter the authentication code sent by Telegram: ";
      std::string code;
      std::cin >> code;

      const std::string payload = std::format(R"({{"code": "{}"}})", code);
      auto res =
          client_->send_request("checkAuthenticationCode", payload, 10.0);
      if (!res) {
        std::cerr << "Invalid code: " << res.error() << std::endl;
      }
    } else if (auth_state_ == "authorizationStateWaitPassword") {
      std::cout << "Enter your 2FA password: ";
      std::string password;
      std::cin >> password;

      const std::string payload =
          std::format(R"({{"password": "{}"}})", password);
      auto res =
          client_->send_request("checkAuthenticationPassword", payload, 10.0);
      if (!res) {
        std::cerr << "Invalid password: " << res.error() << std::endl;
      }
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(500));
  }

  return std::unexpected("Authentication loop timed out");
}

} // namespace grm
