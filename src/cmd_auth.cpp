#include "grm/app.hpp"
#include "grm/json_utils.hpp"
#include "grm/logger.hpp"
#include "grm/qrcodegen.hpp"
#include <chrono>
#include <filesystem>
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
              OptionSpec{"-q", "--qr", "", "Authenticate via QR code (opens /tmp/grm-login-qr.svg via xdg-open)", {}},
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

bool generate_styled_qr_svg(std::string_view link, const std::string &output_path, int width = 500, int margin = 40) {
  try {
    const auto segs = qrcodegen::QrSegment::makeSegments(std::string(link).c_str());
    const auto qr = qrcodegen::QrCode::encodeSegments(
        segs, qrcodegen::QrCode::Ecc::MEDIUM, 1, 40, -1, false);
    const int count = qr.getSize();

    const int available = width - (2 * margin);
    const int cell_size = available / count;
    const int real_margin = (width - (count * cell_size)) / 2;

    std::ofstream out(output_path);
    if (!out.is_open()) {
      return false;
    }

    out << std::format(
        R"(<svg xmlns="http://www.w3.org/2000/svg" version="1.1" width="{0}" height="{0}" viewBox="0 0 {0} {0}">
  <rect width="100%" height="100%" fill="#ffffff" rx="24" ry="24"/>
)", width);

    auto is_corner = [count](int x, int y) {
      if (x < 7 && y < 7) return true;
      if (x >= count - 7 && y < 7) return true;
      if (x < 7 && y >= count - 7) return true;
      return false;
    };

    auto get_neighbor = [&](int x, int y, int dx, int dy) {
      int nx = x + dx;
      int ny = y + dy;
      if (nx < 0 || ny < 0 || nx >= count || ny >= count) return false;
      if (is_corner(nx, ny)) return false;
      return qr.getModule(nx, ny);
    };

    // 1. Draw corner squares (extra-rounded)
    auto draw_corner = [&](int x0, int y0) {
      const double n = real_margin + (x0 * cell_size);
      const double o = real_margin + (y0 * cell_size);
      const double r = 7.0 * cell_size;
      const double i = r / 7.0; // cell_size

      // Outer extra-rounded frame (exact match to qr-code-styling)
      std::string d = std::format(
          "M {:.2f} {:.2f} v {:.2f} a {:.2f} {:.2f}, 0, 0, 0, {:.2f} {:.2f} h {:.2f} a {:.2f} {:.2f}, 0, 0, 0, {:.2f} {:.2f} v {:.2f} a {:.2f} {:.2f}, 0, 0, 0, {:.2f} {:.2f} h {:.2f} a {:.2f} {:.2f}, 0, 0, 0, {:.2f} {:.2f} M {:.2f} {:.2f} h {:.2f} a {:.2f} {:.2f}, 0, 0, 1, {:.2f} {:.2f} v {:.2f} a {:.2f} {:.2f}, 0, 0, 1, {:.2f} {:.2f} h {:.2f} a {:.2f} {:.2f}, 0, 0, 1, {:.2f} {:.2f} v {:.2f} a {:.2f} {:.2f}, 0, 0, 1, {:.2f} {:.2f} Z",
          n, o + 2.5 * i, 2.0 * i, 2.5 * i, 2.5 * i, 2.5 * i, 2.5 * i, 2.0 * i, 2.5 * i, 2.5 * i, 2.5 * i, -2.5 * i, -2.0 * i, 2.5 * i, 2.5 * i, -2.5 * i, -2.5 * i, -2.0 * i, 2.5 * i, 2.5 * i, -2.5 * i, 2.5 * i,
          n + 2.5 * i, o + i, 2.0 * i, 1.5 * i, 1.5 * i, 1.5 * i, 1.5 * i, 2.0 * i, 1.5 * i, 1.5 * i, -1.5 * i, 1.5 * i, -2.0 * i, 1.5 * i, 1.5 * i, -1.5 * i, -1.5 * i, -2.0 * i, 1.5 * i, 1.5 * i, 1.5 * i, -1.5 * i
      );

      out << std::format(
          "  <path fill=\"#000000\" fill-rule=\"evenodd\" clip-rule=\"evenodd\" d=\"{}\"/>\n", d);

      // Inner dot
      const double cx = n + 3.5 * i;
      const double cy = o + 3.5 * i;
      const double cr = 1.5 * i;
      out << std::format("  <circle cx=\"{:.2f}\" cy=\"{:.2f}\" r=\"{:.2f}\" fill=\"#000000\"/>\n", cx, cy, cr);
    };

    draw_corner(0, 0);
    draw_corner(count - 7, 0);
    draw_corner(0, count - 7);

    // 2. Draw rounded connected modules
    for (int y = 0; y < count; ++y) {
      for (int x = 0; x < count; ++x) {
        if (is_corner(x, y)) continue;
        if (!qr.getModule(x, y)) continue;

        const double n = real_margin + (x * cell_size);
        const double o = real_margin + (y * cell_size);
        const double r = static_cast<double>(cell_size);

        const bool left = get_neighbor(x, y, -1, 0);
        const bool right = get_neighbor(x, y, 1, 0);
        const bool top = get_neighbor(x, y, 0, -1);
        const bool bottom = get_neighbor(x, y, 0, 1);

        const int neighbors = (left ? 1 : 0) + (right ? 1 : 0) + (top ? 1 : 0) + (bottom ? 1 : 0);

        const double cx = n + r / 2.0;
        const double cy = o + r / 2.0;

        if (neighbors == 0) {
          // Isolated dot
          out << std::format("  <circle cx=\"{:.2f}\" cy=\"{:.2f}\" r=\"{:.2f}\" fill=\"#000000\"/>\n", cx, cy, r / 2.0);
        } else if (neighbors > 2 || (left && right) || (top && bottom)) {
          // Full square
          out << std::format("  <rect x=\"{:.2f}\" y=\"{:.2f}\" width=\"{:.2f}\" height=\"{:.2f}\" fill=\"#000000\"/>\n", n, o, r, r);
        } else if (neighbors == 2) {
          int rot = 0;
          if (left && top) rot = 90;
          else if (top && right) rot = 180;
          else if (right && bottom) rot = -90;

          std::string d = std::format(
              "M {:.2f} {:.2f} v {:.2f} h {:.2f} v {:.2f} a {:.2f} {:.2f}, 0, 0, 0, {:.2f} {:.2f} Z",
              n, o, r, r, -r / 2.0, r / 2.0, r / 2.0, -r / 2.0, -r / 2.0);

          if (rot != 0) {
            out << std::format("  <path fill=\"#000000\" transform=\"rotate({}, {:.2f}, {:.2f})\" d=\"{}\"/>\n", rot, cx, cy, d);
          } else {
            out << std::format("  <path fill=\"#000000\" d=\"{}\"/>\n", d);
          }
        } else if (neighbors == 1) {
          int rot = 0;
          if (top) rot = 90;
          else if (right) rot = 180;
          else if (bottom) rot = -90;

          std::string d = std::format(
              "M {:.2f} {:.2f} v {:.2f} h {:.2f} a {:.2f} {:.2f}, 0, 0, 0, 0 {:.2f} Z",
              n, o, r, r / 2.0, r / 2.0, r / 2.0, -r);

          if (rot != 0) {
            out << std::format("  <path fill=\"#000000\" transform=\"rotate({}, {:.2f}, {:.2f})\" d=\"{}\"/>\n", rot, cx, cy, d);
          } else {
            out << std::format("  <path fill=\"#000000\" d=\"{}\"/>\n", d);
          }
        }
      }
    }

    out << "</svg>\n";
    out.close();
    return true;
  } catch (...) {
    return false;
  }
}

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
      grm::log::info("Resetting closed session database...");
      client_->stop();
      std::error_code ec;
      std::filesystem::remove_all(config_.db_dir, ec);
      is_closed_ = false;
      {
        std::unique_lock<std::mutex> lock(auth_mutex_);
        auth_state_.clear();
      }
      if (auto r = init_tdlib(); !r) {
        return std::unexpected(r.error());
      }
      last_state.clear();
      continue;
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

          // Generate standalone vector SVG QR Code
          const std::string svg_path = "/tmp/grm-login-qr.svg";
          generate_styled_qr_svg(link, svg_path);

          std::cout << "\n"
                    << "============================================================\n"
                    << " [AUTH] QR CODE AUTHENTICATION (Scan within 30s)\n"
                    << "============================================================\n"
                    << " [AUTH] Vector QR Code generated: " << svg_path << "\n";

          if (std::getenv("DISPLAY") || std::getenv("WAYLAND_DISPLAY")) {
            std::cout << " [AUTH] Opening " << svg_path << " via xdg-open...\n\n"
                      << " 1. Open Telegram on your mobile phone:\n"
                      << "    Settings -> Devices -> Link Desktop Device\n"
                      << " 2. Point your camera at the QR code image.\n";
            std::string cmd = "xdg-open " + svg_path + " >/dev/null 2>&1 &";
            int _ = std::system(cmd.c_str());
            (void)_;
          } else {
            std::cout << " [AUTH] Headless session detected (no graphical desktop environment).\n\n"
                      << " 1. Download or open " << svg_path << " in an image viewer or browser.\n"
                      << " 2. Open Telegram on your mobile phone:\n"
                      << "    Settings -> Devices -> Link Desktop Device\n"
                      << " 3. Point your camera at the QR code image.\n";
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

  // Wait up to 10 seconds for TDLib parameters and database encryption key to settle
  {
    std::unique_lock<std::mutex> lock(auth_mutex_);
    auth_cv_.wait_for(lock, std::chrono::seconds(10), [this] {
      return !auth_state_.empty() &&
             auth_state_ != "authorizationStateWaitTdlibParameters" &&
             auth_state_ != "authorizationStateWaitEncryptionKey";
    });
  }

  const std::string state = get_auth_state();

  if (state == "authorizationStateWaitPhoneNumber" ||
      state == "authorizationStateWaitOtherDeviceConfirmation" ||
      state == "authorizationStateClosed") {
    grm::log::auth("Already logged out (no active session).");
    return 0;
  }

  if (state != "authorizationStateReady") {
    return std::unexpected("Unable to determine authorization state from TDLib (current: " +
                           (state.empty() ? "uninitialized" : state) + ").");
  }

  grm::log::auth("Logging out from Telegram...");
  auto res = client_->send_request("logOut", "{}", 15.0);
  if (!res) {
    if (res.error().find("401") != std::string::npos ||
        res.error().find("Unauthorized") != std::string::npos) {
      grm::log::auth("Session is already unauthorized on Telegram server. Purging local session data...");
      auto destroy_res = client_->send_request("destroy", "{}", 5.0);
      (void)destroy_res;
      client_->stop();
      std::error_code ec;
      std::filesystem::remove_all(config_.db_dir, ec);
      grm::log::auth("Logged out successfully. Local session cleared.");
      return 0;
    }
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

  client_->stop();
  std::error_code ec;
  std::filesystem::remove_all(config_.db_dir, ec);

  grm::log::auth("Logged out successfully. Session closed.");
  return 0;
}

} // namespace grm
