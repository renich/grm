#include "grm/app.hpp"
#include "grm/config.hpp"
#include "grm/logger.hpp"
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
    } else if (arg == "-V" || arg == "--version") {

      options.version = true;
    } else if (arg == "-v" || arg == "--verbose") {
      options.verbosity = grm::log::VerbosityLevel::Verbose;
    } else if (arg == "-d" || arg == "--debug") {
      options.verbosity = grm::log::VerbosityLevel::Debug;
    } else if (arg == "-q" || arg == "--quiet") {
      options.verbosity = grm::log::VerbosityLevel::Quiet;
    } else if ((arg == "-c" || arg == "--config") && i + 1 < argc) {
      options.custom_config_path = argv[++i];
    } else if ((arg == "-p" || arg == "--phone") && i + 1 < argc) {
      options.phone = argv[++i];
    } else if ((arg == "-k" || arg == "--code") && i + 1 < argc) {
      options.code = argv[++i];

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
