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

  for (int i = 1; i < argc; ++i) {
    std::string_view arg(argv[i]);

    if (arg == "-h" || arg == "--help") {
      options.help = true;
      command_args.emplace_back(arg);
    } else if (arg == "-H" || arg == "--help=all") {
      options.help_all = true;
      command_args.emplace_back("--help=all");
    } else if (arg == "-p" || arg == "-P" || arg == "--pretty") {
      options.pretty = true;
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
      } else if (fmt_val == "jsonl" || fmt_val == "ndjson") {
        options.format = grm::fmt::OutputFormat::JsonL;
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
    } else if ((arg == "-N" || arg == "--name-format") && i + 1 < argc) {
      std::string_view nf_val(argv[++i]);
      if (nf_val == "fullname" || nf_val == "full_name" || nf_val == "full") {
        options.name_format = grm::NameFormat::Fullname;
      } else {
        options.name_format = grm::NameFormat::Username;
      }
    } else if (arg.starts_with("--name-format=")) {
      std::string_view nf_val = arg.substr(14);
      if (nf_val == "fullname" || nf_val == "full_name" || nf_val == "full") {
        options.name_format = grm::NameFormat::Fullname;
      } else {
        options.name_format = grm::NameFormat::Username;
      }
    } else if (arg == "-u" || arg == "--username") {
      options.name_format = grm::NameFormat::Username;
    } else if (arg == "--full-name" || arg == "--fullname") {
      options.name_format = grm::NameFormat::Fullname;
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

  grm::Config cfg = std::move(*cfg_res);
  if (options.format == grm::fmt::OutputFormat::Auto &&
      cfg.default_format != grm::fmt::OutputFormat::Auto) {
    options.format = cfg.default_format;
  }
  if (options.color_mode == grm::fmt::ColorMode::Auto &&
      cfg.default_color_mode != grm::fmt::ColorMode::Auto) {
    options.color_mode = cfg.default_color_mode;
  }
  // Default name format from config
  if (options.name_format == grm::NameFormat::Username &&
      cfg.default_name_format != grm::NameFormat::Username) {
    options.name_format = cfg.default_name_format;
  }

  grm::App app(cfg, options);
  auto result = app.run(command_args);

  if (!result) {
    grm::log::error(result.error());
    return 1;
  }

  return *result;
}
