#include "grm/app.hpp"
#include "grm/json_utils.hpp"
#include "grm/logger.hpp"
#include <chrono>
#include <format>
#include <fstream>
#include <iostream>
#include <termios.h>
#include <thread>
#include <unistd.h>

namespace grm {

CommandSpec get_login_spec() {
  return CommandSpec{
      "login",
      "Authenticate Telegram account with TDLib",
      {
          SubcommandSpec{"login", "[-p|--phone <number>] [-k|--code <code>] [-q|--qr]", "Authenticate Telegram session via terminal phone code or browser QR code", {
              OptionSpec{"-p", "--phone", "<number>", "International phone number (e.g. +523330000000)", {}},
              OptionSpec{"-k", "--code", "<code>", "Authentication code received via Telegram or SMS", {}},
              OptionSpec{"-q", "--qr", "", "Authenticate via QR code (opens /tmp/grm-login-qr.html via xdg-open)", {}},
              OptionSpec{"-h", "--help", "", "Show login help message", {}}
          }}
      },
      {}
  };
}

CommandSpec get_logout_spec() {
  return CommandSpec{
      "logout",
      "Log out from Telegram and terminate TDLib session",
      {
          SubcommandSpec{"logout", "[-h|--help]", "Log out from Telegram and clear local session state", {
              OptionSpec{"-h", "--help", "", "Show logout help message", {}}
          }}
      },
      {}
  };
}

namespace {

struct TermiosGuard {
  termios oldt;
  TermiosGuard() {
    tcgetattr(STDIN_FILENO, &oldt);
    termios newt = oldt;
    newt.c_lflag &= static_cast<tcflag_t>(~ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
  }
  ~TermiosGuard() {
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
  }
};

std::string read_secure_password(std::string_view prompt) {
  std::cout << prompt;
  std::cout.flush();

  std::string password;
  {
    TermiosGuard guard;
    std::getline(std::cin, password);
    if (!password.empty() && password.back() == '\r') {
      password.pop_back();
    }
  }

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
  std::string last_qr_link;

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
      } else if (state == "authorizationStateWaitPhoneNumber") {
        if (options_.qr) {
          grm::log::auth("Requesting QR Code authentication from Telegram...");
          auto res = client_->send_request("requestQrCodeAuthentication", "{}", 15.0);
          if (!res) {
            grm::log::error("Failed to request QR Code: " + res.error());
            last_state.clear();
          }
          continue;
        }

        std::string phone = options_.phone;
        if (phone.empty()) {
          std::cout << "\n[AUTH] Enter your Telegram phone number (e.g. +521234567890) or type 'qr' for QR Code login:\n> "
                    << std::flush;
          std::cin >> phone;
        } else {
          grm::log::auth("Using pre-filled phone number: " + phone);
        }

        if (phone == "qr" || phone == "QR") {
          options_.qr = true;
          grm::log::auth("Requesting QR Code authentication from Telegram...");
          auto res = client_->send_request("requestQrCodeAuthentication", "{}", 15.0);
          if (!res) {
            grm::log::error("Failed to request QR Code: " + res.error());
            last_state.clear();
          }
          continue;
        }

        const std::string payload = std::format(
            R"({{
              "phone_number": "{}",
              "settings": {{
                "@type": "phoneNumberAuthenticationSettings",
                "allow_flash_call": false,
                "allow_missed_call": false,
                "is_current_phone_number": false,
                "allow_sms_retriever_api": false,
                "firebase_notification_delivery_token": ""
              }}
            }})",
            escape_json_string(phone));

        grm::log::auth("Submitting phone number to Telegram...");
        auto res = client_->send_request("setAuthenticationPhoneNumber",
                                         payload, 15.0);
        if (!res) {
          if (res.error().find("406") != std::string::npos ||
              res.error().find("UPDATE_APP_TO_LOGIN") != std::string::npos) {
            grm::log::error(
                "Telegram API security restriction [406 UPDATE_APP_TO_LOGIN].");
            grm::log::info(
                "Telegram rejected phone-based authentication for this account. Switching to QR Code login...");
            options_.qr = true;
            auto qr_res = client_->send_request("requestQrCodeAuthentication", "{}", 15.0);
            if (!qr_res) {
              grm::log::error("Failed to request QR Code: " + qr_res.error());
            }
            last_state.clear();
            continue;
          } else {
            grm::log::error("Failed to set phone number: " + res.error());
          }
          options_.phone.clear(); // Clear invalid option
          last_state.clear();
        } else {
          grm::log::auth(
              "Phone number submitted. Waiting for authentication code...");
        }

      } else if (state == "authorizationStateWaitOtherDeviceConfirmation") {
        std::string link = get_qr_link();
        if (link.empty()) {
          if (auto res = client_->send_request("getAuthorizationState", "{}", 2.0)) {
            if (auto direct_link = res->get_string("link")) {
              link = *direct_link;
              set_qr_link(link);
            } else if (auto st = res->get_object("authorization_state")) {
              if (auto nested_link = st->get_string("link")) {
                link = *nested_link;
                set_qr_link(link);
              }
            }
          }
        }

        if (!link.empty() && link != last_qr_link) {
          last_qr_link = link;

          // Generate HTML page using official Telegram Web qr-code-styling engine
          try {
            std::ofstream html_out("/tmp/grm-login-qr.html");
            if (html_out) {
              html_out << R"(<!DOCTYPE html>
<html>
<head>
<meta charset="utf-8">
<meta http-equiv="refresh" content="28">
<title>grm Telegram Login</title>
<script type="text/javascript" src="https://unpkg.com/qr-code-styling@1.5.0/lib/qr-code-styling.js"></script>
<style>
body {
  margin: 0;
  padding: 0;
  display: flex;
  justify-content: center;
  align-items: center;
  min-height: 100vh;
  background-color: #0f141c;
  font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, Helvetica, Arial, sans-serif;
  color: #ffffff;
}
.card {
  background-color: #17212b;
  padding: 36px 48px;
  border-radius: 16px;
  text-align: center;
  box-shadow: 0 10px 30px rgba(0,0,0,0.5);
  max-width: 440px;
}
#qr-container {
  background: #ffffff;
  padding: 24px;
  border-radius: 16px;
  display: inline-block;
  margin: 20px 0;
}
h2 { margin: 0 0 8px; font-size: 22px; font-weight: 600; color: #ffffff; }
p { margin: 4px 0; color: #7f91a4; font-size: 14px; }
.steps { text-align: left; margin-top: 18px; font-size: 14px; line-height: 1.6; color: #c4c9cc; }
</style>
</head>
<body>
<div class="card">
  <h2>Log in to Telegram by QR Code</h2>
  <p>grm CLI Client</p>
  <div id="qr-container"></div>
  <div class="steps">
    1. Open <b>Telegram</b> on your phone<br>
    2. Go to <b>Settings &gt; Devices &gt; Link Desktop Device</b><br>
    3. Point your camera at this QR code
  </div>
</div>
<script>
if (typeof QRCodeStyling !== 'undefined') {
  const qrCode = new QRCodeStyling({
    width: 280,
    height: 280,
    type: "svg",
    data: ")" << link << R"(",
    margin: 0,
    qrOptions: {
      typeNumber: 0,
      mode: "Byte",
      errorCorrectionLevel: "M"
    },
    dotsOptions: {
      color: "#000000",
      type: "rounded"
    },
    cornersSquareOptions: {
      color: "#000000",
      type: "extra-rounded"
    },
    cornersDotOptions: {
      color: "#000000",
      type: "dot"
    }
  });
  qrCode.append(document.getElementById("qr-container"));
}
</script>
</body>
</html>)";
              html_out.close();
            }
          } catch (...) {}

          std::cout << "\n"
                    << "============================================================\n"
                    << " [AUTH] QR CODE AUTHENTICATION (Scan within 30s)\n"
                    << "============================================================\n"
                    << " [AUTH] HTML QR code page generated: /tmp/grm-login-qr.html\n";

          if (std::getenv("DISPLAY") || std::getenv("WAYLAND_DISPLAY")) {
            std::cout << " [AUTH] Opening /tmp/grm-login-qr.html via xdg-open...\n\n"
                      << " 1. Open Telegram on your mobile phone:\n"
                      << "    Settings -> Devices -> Link Desktop Device\n"
                      << " 2. Point your camera at the QR code in your browser window.\n";
            int _ = std::system("xdg-open /tmp/grm-login-qr.html >/dev/null 2>&1 &");
            (void)_;
          } else {
            std::cout << " [AUTH] Headless session detected (no graphical desktop environment).\n\n"
                      << " 1. Download or open /tmp/grm-login-qr.html in a web browser.\n"
                      << " 2. Open Telegram on your mobile phone:\n"
                      << "    Settings -> Devices -> Link Desktop Device\n"
                      << " 3. Point your camera at the QR code displayed in your browser.\n";
          }

          std::cout << "============================================================\n"
                    << " Waiting for confirmation from your Telegram device...\n"
                    << std::flush;
        }

        last_state.clear(); // Re-check on every tick so new tokens are rendered immediately

      } else if (state == "authorizationStateWaitCode") {
        std::string code = options_.code;
        if (code.empty()) {
          std::cout << "\n[AUTH] Enter authentication code (or type 'resend' for SMS):\n> "
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

std::expected<int, std::string> App::cmd_logout() {
  if (config_.api_id == 0 || config_.api_hash.empty()) {
    return std::unexpected(
        "Missing Telegram API credentials. Please set 'api_id' and 'api_hash' in "
        "~/.config/grm/config.json (or pass via -c/--config).");
  }

  if (auto res = init_tdlib(); !res) {
    return std::unexpected(res.error());
  }

  // Wait briefly for TDLib parameters to be negotiated and initial auth state determined
  {
    std::unique_lock<std::mutex> lock(auth_mutex_);
    auth_cv_.wait_for(lock, std::chrono::seconds(3), [this] {
      return !auth_state_.empty() &&
             auth_state_ != "authorizationStateWaitTdlibParameters";
    });
  }

  const std::string state = get_auth_state();

  if (state != "authorizationStateReady") {
    grm::log::auth("Already logged out (no active session).");
    return 0;
  }

  grm::log::auth("Logging out from Telegram...");
  auto res = client_->send_request("logOut", "{}", 15.0);
  if (!res) {
    return std::unexpected("Failed to log out: " + res.error());
  }

  // Wait up to 5 seconds for TDLib to close session
  auto start = std::chrono::steady_clock::now();
  while (std::chrono::steady_clock::now() - start < std::chrono::seconds(5)) {
    if (get_auth_state() == "authorizationStateClosed") {
      break;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }

  grm::log::auth("Logged out successfully. Session closed.");
  return 0;
}

} // namespace grm
