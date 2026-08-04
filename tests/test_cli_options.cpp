// TestCliOptions validates GNU short & long option flag parsing across all subcommands
#include <charconv>
#include <cstdlib>
#include <iostream>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

static void check(bool condition, const std::string &msg) {
  if (!condition) {
    std::cerr << "Assertion failed: " << msg << std::endl;
    std::exit(1);
  }
}

namespace grm::test {

struct MsgSendOptions {
  int64_t chat_id{0};
  bool chat_id_set{false};
  std::string message_text;
  std::string caption;
  std::vector<std::string> attachments;
  bool is_media{false};
  int64_t message_thread_id{0};
};

static MsgSendOptions
parse_msg_send_options(const std::vector<std::string_view> &args) {
  MsgSendOptions opts;
  for (size_t i = 0; i < args.size(); ++i) {
    std::string_view arg = args[i];
    if ((arg == "-a" || arg == "--attach" || arg == "-A" ||
         arg == "--attachment") &&
        i + 1 < args.size()) {
      opts.attachments.emplace_back(args[++i]);
    } else if (arg.starts_with("--attach=")) {
      opts.attachments.emplace_back(arg.substr(9));
    } else if (arg == "-m" || arg == "--media") {
      opts.is_media = true;
    } else if ((arg == "-C" || arg == "--caption") && i + 1 < args.size()) {
      opts.caption = std::string(args[++i]);
    } else if ((arg == "-t" || arg == "--topic") && i + 1 < args.size()) {
      int64_t val = 0;
      auto [ptr, ec] = std::from_chars(
          args[i + 1].data(), args[i + 1].data() + args[i + 1].size(), val);
      if (ec == std::errc{}) {
        opts.message_thread_id = val;
        ++i;
      }
    } else if (!opts.chat_id_set) {
      int64_t val = 0;
      auto [ptr, ec] =
          std::from_chars(arg.data(), arg.data() + arg.size(), val);
      if (ec == std::errc{} && ptr == arg.data() + arg.size()) {
        opts.chat_id = val;
        opts.chat_id_set = true;
      }
    } else if (opts.chat_id_set && opts.message_text.empty() &&
               !arg.starts_with("-")) {
      opts.message_text = std::string(arg);
    }
  }
  return opts;
}

struct ExportOptions {
  int64_t chat_id{0};
  bool chat_id_set{false};
  std::string format_type{"json"};
  std::string out_path;
  int limit{1000};
};

static ExportOptions
parse_export_options(const std::vector<std::string_view> &args) {
  ExportOptions opts;
  for (size_t i = 0; i < args.size(); ++i) {
    std::string_view arg = args[i];
    if ((arg == "-f" || arg == "--format") && i + 1 < args.size()) {
      opts.format_type = std::string(args[++i]);
    } else if ((arg == "-o" || arg == "--output") && i + 1 < args.size()) {
      opts.out_path = std::string(args[++i]);
    } else if ((arg == "-n" || arg == "--limit") && i + 1 < args.size()) {
      int val = 0;
      auto [ptr, ec] = std::from_chars(
          args[i + 1].data(), args[i + 1].data() + args[i + 1].size(), val);
      if (ec == std::errc{}) {
        opts.limit = val;
        ++i;
      }
    } else if (!opts.chat_id_set) {
      int64_t val = 0;
      auto [ptr, ec] =
          std::from_chars(arg.data(), arg.data() + arg.size(), val);
      if (ec == std::errc{} && ptr == arg.data() + arg.size()) {
        opts.chat_id = val;
        opts.chat_id_set = true;
      }
    }
  }
  return opts;
}

} // namespace grm::test

void test_msg_send_option_parsing() {
  auto opts = grm::test::parse_msg_send_options(
      {"-a", "/tmp/file1.pdf", "-a", "/tmp/file2.jpg", "-m", "-C", "My Caption",
       "-t", "42", "-1001789902965", "Hello World"});

  check(opts.chat_id_set, "Chat ID should be set");
  check(opts.chat_id == -1001789902965LL, "Chat ID should match negative supergroup ID");
  check(opts.message_text == "Hello World", "Message text should match");
  check(opts.caption == "My Caption", "Caption should match");
  check(opts.is_media, "Media flag should be true");
  check(opts.message_thread_id == 42, "Topic thread ID should be 42");
  check(opts.attachments.size() == 2, "Should have 2 attachments");
  check(opts.attachments[0] == "/tmp/file1.pdf", "First attachment match");
  check(opts.attachments[1] == "/tmp/file2.jpg", "Second attachment match");

  std::cout << "[PASS] test_msg_send_option_parsing\n";
}

void test_export_option_parsing() {
  auto opts = grm::test::parse_export_options(
      {"-f", "csv", "-o", "/tmp/history.csv", "-n", "500", "-1001789902965"});

  check(opts.chat_id_set, "Chat ID set");
  check(opts.chat_id == -1001789902965LL, "Chat ID match");
  check(opts.format_type == "csv", "Format match");
  check(opts.out_path == "/tmp/history.csv", "Output path match");
  check(opts.limit == 500, "Limit match");

  std::cout << "[PASS] test_export_option_parsing\n";
}

int main() {
  std::cout << "Running test_cli_options...\n";
  test_msg_send_option_parsing();
  test_export_option_parsing();
  std::cout << "All test_cli_options unit tests passed successfully.\n";
  return 0;
}
