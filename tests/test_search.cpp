#include "grm/app.hpp"
#include "grm/command_registry.hpp"
#include "grm/formatter.hpp"
#include <cassert>
#include <iostream>
#include <sstream>

void test_search_spec_registration() {
  const auto &registry = grm::CommandRegistry::get_instance();
  (void)registry;
  auto spec = grm::App::get_search_spec();


  assert(spec.name == "search");
  assert(!spec.subcommands.empty());

  bool has_chats = false;
  bool has_supergroups = false;
  bool has_msgs = false;
  bool has_users = false;

  for (const auto &sub : spec.subcommands) {
    if (sub.name == "chats") has_chats = true;
    if (sub.name == "supergroups") has_supergroups = true;
    if (sub.name == "msgs") has_msgs = true;
    if (sub.name == "users") has_users = true;
  }

  assert(has_chats);
  assert(has_supergroups);
  assert(has_msgs);
  assert(has_users);

  (void)has_chats;
  (void)has_supergroups;
  (void)has_msgs;
  (void)has_users;


  std::cout << "[TEST PASS] test_search_spec_registration\n";
}

void test_search_formatter() {
  grm::fmt::SearchSummary summary;
  summary.query = "linux";

  grm::fmt::ChatItem c1;
  c1.id = -100123456789;
  c1.title = "Linux Kernel Devs";
  c1.type = "supergroup";
  summary.chats.push_back(c1);

  grm::fmt::UserItem u1;
  u1.id = 12345678;
  u1.first_name = "Linus";
  u1.last_name = "Torvalds";
  u1.username = "torvalds";
  u1.status = "Online";
  summary.users.push_back(u1);

  grm::fmt::MessageItem m1;
  m1.id = 555;
  m1.chat_id = -100123456789;
  m1.date = 1750000000;
  m1.sender = "torvalds";
  m1.text = "Fedora Linux Release 42 is live!";
  summary.messages.push_back(m1);

  std::ostringstream ss_human;
  grm::fmt::Formatter::print_search_summary(summary, grm::fmt::OutputFormat::Plain, grm::fmt::ColorMode::Never, ss_human, false);
  std::string human_out = ss_human.str();
  assert(human_out.find("SEARCH RESULTS FOR: \"linux\"") != std::string::npos);
  assert(human_out.find("Linux Kernel Devs") != std::string::npos);
  assert(human_out.find("Linus Torvalds") != std::string::npos);
  assert(human_out.find("Fedora Linux Release 42 is live!") != std::string::npos);

  std::ostringstream ss_json;
  grm::fmt::Formatter::print_search_summary(summary, grm::fmt::OutputFormat::Json, grm::fmt::ColorMode::Never, ss_json, false);
  std::string json_out = ss_json.str();
  assert(json_out.find("\"query\": \"linux\"") != std::string::npos);
  assert(json_out.find("\"chats_count\": 1") != std::string::npos);

  std::cout << "[TEST PASS] test_search_formatter\n";
}

int main() {
  test_search_spec_registration();
  test_search_formatter();
  std::cout << "All search unit tests passed successfully!\n";
  return 0;
}
