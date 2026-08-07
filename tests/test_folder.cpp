#include "grm/app.hpp"
#include "grm/command_registry.hpp"
#include "grm/formatter.hpp"
#include <cassert>
#include <iostream>

void test_folder_spec_registration() {
  auto spec = grm::get_folder_spec();
  assert(spec.name == "folder");
  assert(!spec.subcommands.empty());

  bool has_ls = false;
  bool has_create = false;
  bool has_edit = false;
  bool has_delete = false;

  for (const auto &sub : spec.subcommands) {
    if (sub.name == "ls") has_ls = true;
    if (sub.name == "create") has_create = true;
    if (sub.name == "edit") has_edit = true;
    if (sub.name == "delete") has_delete = true;
  }

  assert(has_ls);
  assert(has_create);
  assert(has_edit);
  assert(has_delete);

  (void)has_ls;
  (void)has_create;
  (void)has_edit;
  (void)has_delete;

  std::cout << "[TEST PASS] test_folder_spec_registration\n";

}

void test_folder_formatter() {
  std::vector<grm::fmt::ChatFolderSummary> folders;
  grm::fmt::ChatFolderSummary f1;
  f1.id = 1;
  f1.title = "Work";
  f1.icon = "Briefcase";
  f1.include_groups = true;
  f1.include_channels = true;
  f1.pinned_chat_ids = {-1001234567890LL};
  f1.included_chat_ids = {-1001234567890LL, -1009876543210LL};
  folders.push_back(f1);

  std::stringstream ss;
  grm::fmt::Formatter::print_folders(folders, grm::fmt::OutputFormat::Human, grm::fmt::ColorMode::Never, ss, true);
  std::string output = ss.str();

  assert(output.find("Work") != std::string::npos);
  assert(output.find("Groups") != std::string::npos);
  assert(output.find("Channels") != std::string::npos);

  std::cout << "[TEST PASS] test_folder_formatter\n";
}

int main() {
  test_folder_spec_registration();
  test_folder_formatter();
  std::cout << "All test_folder assertions passed successfully!\n";
  return 0;
}
