#include "grm/cmd_story.hpp"
#include "grm/json_utils.hpp"
#include <cstdlib>
#include <iostream>

static void test_assert(bool cond, const char *msg, int line) {
  if (!cond) {
    std::cerr << "Assertion failed at line " << line << ": " << msg << "\n";
    std::abort();
  }
}

#define TEST_ASSERT(cond) test_assert((cond), #cond, __LINE__)

void test_period_parsing() {
  auto p1 = grm::parse_period_string("6h");
  TEST_ASSERT(p1.has_value() && *p1 == 21600);

  auto p2 = grm::parse_period_string("12h");
  TEST_ASSERT(p2.has_value() && *p2 == 43200);

  auto p3 = grm::parse_period_string("24h");
  TEST_ASSERT(p3.has_value() && *p3 == 86400);

  auto p4 = grm::parse_period_string("1d");
  TEST_ASSERT(p4.has_value() && *p4 == 86400);

  auto p5 = grm::parse_period_string("48h");
  TEST_ASSERT(p5.has_value() && *p5 == 172800);

  auto p6 = grm::parse_period_string("2d");
  TEST_ASSERT(p6.has_value() && *p6 == 172800);

  auto p_empty = grm::parse_period_string("");
  TEST_ASSERT(p_empty.has_value() && *p_empty == 86400);

  auto p_invalid = grm::parse_period_string("5h");
  TEST_ASSERT(!p_invalid.has_value());

  std::cout << "[PASS] test_period_parsing\n";
}

void test_story_post_arg_parsing() {
  {
    grm::StoryPostOptions opts;
    std::string err;
    std::vector<std::string> args = {"--photo",   "test.jpg",  "--caption",
                                     "Hello",     "--privacy", "contacts",
                                     "--period",  "12h",       "--pinned",
                                     "--protect", "--chat",    "12345"};
    bool ok = grm::parse_story_post_args(args, opts, err);
    TEST_ASSERT(ok);
    TEST_ASSERT(opts.photo_path == "test.jpg");
    TEST_ASSERT(opts.video_path.empty());
    TEST_ASSERT(opts.caption == "Hello");
    TEST_ASSERT(opts.privacy == "contacts");
    TEST_ASSERT(opts.active_period == 43200);
    TEST_ASSERT(opts.is_pinned);
    TEST_ASSERT(opts.protect_content);
    TEST_ASSERT(opts.chat_id == 12345);
  }

  {
    grm::StoryPostOptions opts;
    std::string err;
    std::vector<std::string> args = {"--video", "clip.mp4", "--privacy",
                                     "close_friends"};
    bool ok = grm::parse_story_post_args(args, opts, err);
    TEST_ASSERT(ok);
    TEST_ASSERT(opts.video_path == "clip.mp4");
    TEST_ASSERT(opts.photo_path.empty());
    TEST_ASSERT(opts.privacy == "close_friends");
    TEST_ASSERT(opts.active_period == 86400);
  }

  {
    // Both photo and video error
    grm::StoryPostOptions opts;
    std::string err;
    std::vector<std::string> args = {"--photo", "test.jpg", "--video",
                                     "test.mp4"};
    bool ok = grm::parse_story_post_args(args, opts, err);
    TEST_ASSERT(!ok);
    TEST_ASSERT(!err.empty());
  }

  {
    // Neither photo nor video error
    grm::StoryPostOptions opts;
    std::string err;
    std::vector<std::string> args = {"--caption", "No media"};
    bool ok = grm::parse_story_post_args(args, opts, err);
    TEST_ASSERT(!ok);
    TEST_ASSERT(!err.empty());
  }

  {
    // Invalid privacy
    grm::StoryPostOptions opts;
    std::string err;
    std::vector<std::string> args = {"--photo", "test.jpg", "--privacy",
                                     "public_only"};
    bool ok = grm::parse_story_post_args(args, opts, err);
    TEST_ASSERT(!ok);
  }

  std::cout << "[PASS] test_story_post_arg_parsing\n";
}

void test_story_ls_and_delete_args() {
  {
    grm::StoryListOptions opts;
    std::string err;
    std::vector<std::string> args = {"--chat", "-100123456", "--limit", "50"};
    bool ok = grm::parse_story_ls_args(args, opts, err);
    TEST_ASSERT(ok);
    TEST_ASSERT(opts.chat_id == -100123456);
    TEST_ASSERT(!opts.pinned);
    TEST_ASSERT(!opts.archived);
    TEST_ASSERT(!opts.all);
  }

  {
    grm::StoryListOptions opts;
    std::string err;
    std::vector<std::string> args = {"-p", "-n", "10"};
    bool ok = grm::parse_story_ls_args(args, opts, err);
    TEST_ASSERT(ok);
    TEST_ASSERT(opts.pinned);
    TEST_ASSERT(!opts.archived);
    TEST_ASSERT(!opts.all);
    TEST_ASSERT(opts.limit == 10);
  }

  {
    grm::StoryListOptions opts;
    std::string err;
    std::vector<std::string> args = {"--all", "-a"};
    bool ok = grm::parse_story_ls_args(args, opts, err);
    TEST_ASSERT(ok);
    TEST_ASSERT(opts.all);
    TEST_ASSERT(opts.archived);
  }

  {
    grm::StoryEditOptions opts;
    std::string err;
    std::vector<std::string> args = {"--story-id",  "42",      "--caption",
                                     "New Caption", "--photo", "/tmp/test.jpg",
                                     "--chat",      "98765"};
    bool ok = grm::parse_story_edit_args(args, opts, err);
    TEST_ASSERT(ok);
    TEST_ASSERT(opts.story_id == 42);
    TEST_ASSERT(opts.caption == "New Caption");
    TEST_ASSERT(opts.has_caption);
    TEST_ASSERT(opts.photo_path == "/tmp/test.jpg");
    TEST_ASSERT(opts.chat_id == 98765);
  }

  {
    // Missing story ID
    grm::StoryEditOptions opts;
    std::string err;
    std::vector<std::string> args = {"--caption", "New Caption"};
    bool ok = grm::parse_story_edit_args(args, opts, err);
    TEST_ASSERT(!ok);
  }

  {
    // Missing edit payload
    grm::StoryEditOptions opts;
    std::string err;
    std::vector<std::string> args = {"--story-id", "42"};
    bool ok = grm::parse_story_edit_args(args, opts, err);
    TEST_ASSERT(!ok);
  }

  {
    grm::StoryDeleteOptions opts;
    std::string err;
    std::vector<std::string> args = {"--story-id", "42", "--chat", "98765"};
    bool ok = grm::parse_story_delete_args(args, opts, err);
    TEST_ASSERT(ok);
    TEST_ASSERT(opts.story_id == 42);
    TEST_ASSERT(opts.chat_id == 98765);
  }

  {
    // Missing story ID
    grm::StoryDeleteOptions opts;
    std::string err;
    std::vector<std::string> args = {"--chat", "98765"};
    bool ok = grm::parse_story_delete_args(args, opts, err);
    TEST_ASSERT(!ok);
  }

  std::cout << "[PASS] test_story_ls_and_delete_args\n";
}

void test_story_json_builders() {
  grm::StoryPostOptions opts;
  opts.photo_path = "/tmp/image.png";
  opts.caption = "Test Caption";
  opts.privacy = "everyone";
  opts.active_period = 86400;
  opts.is_pinned = false;
  opts.protect_content = true;

  std::string caption_json =
      R"({"@type":"formattedText","text":"Test Caption","entities":[]})";
  std::string post_json =
      grm::build_post_story_json(opts, 112233, caption_json);

  auto parsed = grm::JsonValue::parse(post_json);
  if (!parsed.has_value()) {
    TEST_ASSERT(false);
    return;
  }
  TEST_ASSERT(parsed->get_int("chat_id").value_or(0) == 112233);
  TEST_ASSERT(parsed->get_int("active_period").value_or(0) == 86400);
  TEST_ASSERT(parsed->get_bool("protect_content").value_or(false));
  TEST_ASSERT(!parsed->get_bool("is_posted_to_chat_page").value_or(true));

  auto privacy_obj = parsed->get_object("privacy_settings");
  if (!privacy_obj.has_value()) {
    TEST_ASSERT(false);
    return;
  }
  TEST_ASSERT(privacy_obj->get_string("@type").value_or("") ==
              "storyPrivacySettingsEveryone");

  auto content_obj = parsed->get_object("content");
  if (!content_obj.has_value()) {
    TEST_ASSERT(false);
    return;
  }
  TEST_ASSERT(content_obj->get_string("@type").value_or("") ==
              "inputStoryContentPhoto");

  // Test build_edit_story_json
  grm::StoryEditOptions edit_opts;
  edit_opts.story_id = 42;
  edit_opts.caption = "Updated Caption";
  edit_opts.has_caption = true;
  std::string edit_caption_json =
      R"({"@type":"formattedText","text":"Updated Caption","entities":[]})";
  std::string edit_json =
      grm::build_edit_story_json(112233, 42, edit_opts, edit_caption_json);
  auto parsed_edit = grm::JsonValue::parse(edit_json);
  TEST_ASSERT(parsed_edit.has_value());
  TEST_ASSERT(parsed_edit->get_int("story_poster_chat_id").value_or(0) ==
              112233);
  TEST_ASSERT(parsed_edit->get_int("story_id").value_or(0) == 42);

  std::string del_json = grm::build_delete_story_json(112233, 42);
  auto parsed_del = grm::JsonValue::parse(del_json);
  if (!parsed_del.has_value()) {
    TEST_ASSERT(false);
    return;
  }
  TEST_ASSERT(parsed_del->get_int("story_poster_chat_id").value_or(0) ==
              112233);
  TEST_ASSERT(parsed_del->get_int("story_id").value_or(0) == 42);

  std::cout << "[PASS] test_story_json_builders\n";
}

int main() {
  test_period_parsing();
  test_story_post_arg_parsing();
  test_story_ls_and_delete_args();
  test_story_json_builders();
  std::cout << "All Story unit tests passed successfully!\n";
  return 0;
}
