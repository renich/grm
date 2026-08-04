#include "grm/app.hpp"
#include "grm/config.hpp"
#include <iostream>
#include <vector>

int main(int argc, char *argv[]) {
  std::vector<std::string> args;
  args.reserve(static_cast<size_t>(argc > 1 ? argc - 1 : 0));
  for (int i = 1; i < argc; ++i) {
    args.emplace_back(argv[i]);
  }

  auto cfg_res = grm::Config::load();
  if (!cfg_res) {
    std::cerr << "Configuration Error: " << cfg_res.error() << '\n';
    return 1;
  }

  grm::App app(std::move(*cfg_res));
  auto result = app.run(args);

  if (!result) {
    std::cerr << "Error: " << result.error() << '\n';
    return 1;
  }


  return *result;
}
