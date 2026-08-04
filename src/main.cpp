#include "grm/app.hpp"
#include "grm/config.hpp"
#include "grm/logger.hpp"
#include <algorithm>
#include <array>
#include <iostream>
#include <string_view>
#include <vector>


int main(int argc, char *argv[]) {
  grm::CliOptions options;
  std::vector<std::string> command_args;

  const std::array<std::string_view, 5> subcommands = {
      "login", "chat", "msg", "send", "topic"};


  for (int i = 1; i < argc; ++i) {
    std::string_view arg(argv[i]);

    if (std::ranges::find(subcommands, arg) != subcommands.end()) {
      for (; i < argc; ++i) {
        command_args.emplace_back(argv[i]);
      }
      break;
    }

    if (arg == "-h" || arg == "--help") {
      options.help = true;
      command_args.emplace_back(arg);
    } else if (arg == "-V" || arg == "--version") {
      options.version = true;
    } else if (arg == "-v" || arg == "--verbose") {
      options.verbosity = grm::log::VerbosityLevel::Verbose;
    } else if (arg == "-d" || arg == "--debug") {
      options.verbosity = grm::log::VerbosityLevel::Debug;
    } else if (arg == "-q" || arg == "--quiet") {
      options.verbosity = grm::log::VerbosityLevel::Quiet;
    } else if (arg == "-T" || arg == "--test-dc") {
      options.use_test_dc = true;
    } else if ((arg == "-c" || arg == "--config") && i + 1 < argc) {
      options.custom_config_path = argv[++i];
    } else if ((arg == "-F" || arg == "--format") && i + 1 < argc) {
      std::string_view fmt_val(argv[++i]);
      if (fmt_val == "human") {
        options.format = grm::fmt::OutputFormat::Human;
      } else if (fmt_val == "markdown") {
        options.format = grm::fmt::OutputFormat::Markdown;
      } else if (fmt_val == "json") {
        options.format = grm::fmt::OutputFormat::Json;
      } else if (fmt_val == "plain") {
        options.format = grm::fmt::OutputFormat::Plain;
      }
    } else if (arg == "--color" && i + 1 < argc) {
      std::string_view color_val(argv[++i]);
      if (color_val == "always") {
        options.color_mode = grm::fmt::ColorMode::Always;
      } else if (color_val == "never") {
        options.color_mode = grm::fmt::ColorMode::Never;
      } else if (color_val == "auto") {
        options.color_mode = grm::fmt::ColorMode::Auto;
      }
    } else if (arg == "--no-color") {
      options.color_mode = grm::fmt::ColorMode::Never;
    } else {
      command_args.emplace_back(arg);
    }
  }


  grm::log::set_verbosity(options.verbosity);

  auto cfg_res = grm::Config::load(options.custom_config_path);
  if (!cfg_res) {
    grm::log::error("Configuration Error: " + cfg_res.error());
    return 1;
  }

  grm::App app(std::move(*cfg_res), options);
  auto result = app.run(command_args);

  if (!result) {
    grm::log::error(result.error());
    return 1;
  }

  return *result;
}
