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

void test_story_info_and_viewers_args() {
  {
    grm::StoryInfoOptions opts;
    std::string err;
    std::vector<std::string> args = {"--story-id", "101", "-C", "12345"};
    bool ok = grm::parse_story_info_args(args, opts, err);
    TEST_ASSERT(ok);
    TEST_ASSERT(opts.story_id == 101);
    TEST_ASSERT(opts.chat_id == 12345);
  }

  {
    grm::StoryInfoOptions opts;
    std::string err;
    std::vector<std::string> args = {"102"};
    bool ok = grm::parse_story_info_args(args, opts, err);
    TEST_ASSERT(ok);
    TEST_ASSERT(opts.story_id == 102);
  }

  {
    grm::StoryViewersOptions opts;
    std::string err;
    std::vector<std::string> args = {"--story-id", "101",   "--limit", "25",
                                     "--query",    "alice", "-C",      "12345"};
    bool ok = grm::parse_story_viewers_args(args, opts, err);
    TEST_ASSERT(ok);
    TEST_ASSERT(opts.story_id == 101);
    TEST_ASSERT(opts.limit == 25);
    TEST_ASSERT(opts.query == "alice");
    TEST_ASSERT(opts.chat_id == 12345);
  }

  std::cout << "[PASS] test_story_info_and_viewers_args\n";
}

void test_story_pin_react_privacy_args() {
  {
    grm::StoryPinOptions opts;
    std::string err;
    std::vector<std::string> args = {"--story-id", "55", "-C", "777"};
    bool ok = grm::parse_story_pin_args(args, opts, err, true);
    TEST_ASSERT(ok);
    TEST_ASSERT(opts.story_id == 55);
    TEST_ASSERT(opts.chat_id == 777);
    TEST_ASSERT(opts.is_pinned);
  }

  {
    grm::StoryPinOptions opts;
    std::string err;
    std::vector<std::string> args = {"55"};
    bool ok = grm::parse_story_pin_args(args, opts, err, false);
    TEST_ASSERT(ok);
    TEST_ASSERT(opts.story_id == 55);
    TEST_ASSERT(!opts.is_pinned);
  }

  {
    grm::StoryReactOptions opts;
    std::string err;
    std::vector<std::string> args = {"--story-id", "55", "--emoji",
                                     "🔥",         "-C", "777"};
    bool ok = grm::parse_story_react_args(args, opts, err);
    TEST_ASSERT(ok);
    TEST_ASSERT(opts.story_id == 55);
    TEST_ASSERT(opts.emoji == "🔥");
    TEST_ASSERT(opts.chat_id == 777);
  }

  {
    grm::StoryPrivacyOptions opts;
    std::string err;
    std::vector<std::string> args = {"--story-id", "55", "--privacy",
                                     "close_friends"};
    bool ok = grm::parse_story_privacy_args(args, opts, err);
    TEST_ASSERT(ok);
    TEST_ASSERT(opts.story_id == 55);
    TEST_ASSERT(opts.privacy == "close_friends");
  }

  std::cout << "[PASS] test_story_pin_react_privacy_args\n";
}

void test_story_stickers_and_actions_json() {
  // Test story areas in post
  grm::StoryPostOptions opts;
  opts.photo_path = "/tmp/img.png";
  opts.link_url = "https://gitlab.com/renich/grm";
  opts.reaction_emoji = "🚀";
  std::string post_json = grm::build_post_story_json(opts, 999, "");
  auto parsed_post = grm::JsonValue::parse(post_json);
  TEST_ASSERT(parsed_post.has_value());
  auto areas_obj = parsed_post->get_object("areas");
  TEST_ASSERT(areas_obj.has_value());
  auto areas_arr = areas_obj->get_array("areas");
  TEST_ASSERT(areas_arr.size() == 2);

  // Test toggle pin JSON
  std::string pin_json =
      grm::build_toggle_story_is_posted_to_chat_page_json(999, 42, true);
  auto parsed_pin = grm::JsonValue::parse(pin_json);
  TEST_ASSERT(parsed_pin.has_value());
  TEST_ASSERT(parsed_pin->get_bool("is_posted_to_chat_page").value_or(false));

  // Test set reaction JSON
  std::string react_json = grm::build_set_story_reaction_json(999, 42, "🔥");
  auto parsed_react = grm::JsonValue::parse(react_json);
  TEST_ASSERT(parsed_react.has_value());
  auto rx_obj = parsed_react->get_object("reaction_type");
  TEST_ASSERT(rx_obj.has_value());
  TEST_ASSERT(rx_obj->get_string("emoji").value_or("") == "🔥");

  // Test video post JSON
  grm::StoryPostOptions vid_opts;
  vid_opts.video_path = "/tmp/clip.mp4";
  vid_opts.privacy = "close_friends";
  vid_opts.active_period = 43200;
  std::string vid_json = grm::build_post_story_json(vid_opts, 123, "");
  auto parsed_vid = grm::JsonValue::parse(vid_json);
  TEST_ASSERT(parsed_vid.has_value());
  auto vid_content = parsed_vid->get_object("content");
  TEST_ASSERT(vid_content.has_value());
  TEST_ASSERT(vid_content->get_string("@type").value_or("") ==
              "inputStoryContentVideo");
  auto vid_priv = parsed_vid->get_object("privacy_settings");
  TEST_ASSERT(vid_priv.has_value());
  TEST_ASSERT(vid_priv->get_string("@type").value_or("") ==
              "storyPrivacySettingsCloseFriends");

  // Test video edit JSON
  grm::StoryEditOptions edit_vid_opts;
  edit_vid_opts.video_path = "/tmp/clip2.mp4";
  std::string edit_vid_json =
      grm::build_edit_story_json(123, 77, edit_vid_opts, "");
  auto parsed_edit_vid = grm::JsonValue::parse(edit_vid_json);
  TEST_ASSERT(parsed_edit_vid.has_value());
  auto edit_vid_content = parsed_edit_vid->get_object("content");
  TEST_ASSERT(edit_vid_content.has_value());
  TEST_ASSERT(edit_vid_content->get_string("@type").value_or("") ==
              "inputStoryContentVideo");

  // Test clear reaction JSON
  std::string clear_rx_json = grm::build_set_story_reaction_json(999, 42, "");
  auto parsed_clear_rx = grm::JsonValue::parse(clear_rx_json);
  TEST_ASSERT(parsed_clear_rx.has_value());
  TEST_ASSERT(parsed_clear_rx->get_object("reaction_type") == std::nullopt);

  std::cout << "[PASS] test_story_stickers_and_actions_json\n";
}

void test_story_equals_flag_parsing() {
  {
    grm::StoryPostOptions opts;
    std::string err;
    std::vector<std::string> args = {"--photo=/tmp/pic.jpg",
                                     "--caption=hello",
                                     "--link=https://example.com",
                                     "--reaction=🔥",
                                     "--privacy=contacts",
                                     "--period=2d",
                                     "--chat=12345"};
    bool ok = grm::parse_story_post_args(args, opts, err);
    TEST_ASSERT(ok);
    TEST_ASSERT(opts.photo_path == "/tmp/pic.jpg");
    TEST_ASSERT(opts.caption == "hello");
    TEST_ASSERT(opts.link_url == "https://example.com");
    TEST_ASSERT(opts.reaction_emoji == "🔥");
    TEST_ASSERT(opts.privacy == "contacts");
    TEST_ASSERT(opts.active_period == 172800);
    TEST_ASSERT(opts.chat_id == 12345);
  }

  {
    grm::StoryEditOptions opts;
    std::string err;
    std::vector<std::string> args = {
        "--story-id=88",  "--video=/tmp/vid.mp4",
        "--caption=edit", "--link=https://example.com",
        "--reaction=🚀",  "--chat=99"};
    bool ok = grm::parse_story_edit_args(args, opts, err);
    TEST_ASSERT(ok);
    TEST_ASSERT(opts.story_id == 88);
    TEST_ASSERT(opts.video_path == "/tmp/vid.mp4");
    TEST_ASSERT(opts.caption == "edit");
    TEST_ASSERT(opts.has_caption);
    TEST_ASSERT(opts.link_url == "https://example.com");
    TEST_ASSERT(opts.reaction_emoji == "🚀");
    TEST_ASSERT(opts.has_areas);
    TEST_ASSERT(opts.chat_id == 99);
  }

  {
    grm::StoryListOptions opts;
    std::string err;
    std::vector<std::string> args = {"--chat=777", "--limit=30"};
    bool ok = grm::parse_story_ls_args(args, opts, err);
    TEST_ASSERT(ok);
    TEST_ASSERT(opts.chat_id == 777);
    TEST_ASSERT(opts.limit == 30);
  }

  {
    grm::StoryViewersOptions opts;
    std::string err;
    std::vector<std::string> args = {"--story-id=10", "--query=bob",
                                     "--limit=15", "--chat=55"};
    bool ok = grm::parse_story_viewers_args(args, opts, err);
    TEST_ASSERT(ok);
    TEST_ASSERT(opts.story_id == 10);
    TEST_ASSERT(opts.query == "bob");
    TEST_ASSERT(opts.limit == 15);
    TEST_ASSERT(opts.chat_id == 55);
  }

  {
    grm::StoryPinOptions opts;
    std::string err;
    std::vector<std::string> args = {"--story-id=20", "--chat=33"};
    bool ok = grm::parse_story_pin_args(args, opts, err, true);
    TEST_ASSERT(ok);
    TEST_ASSERT(opts.story_id == 20);
    TEST_ASSERT(opts.chat_id == 33);
  }

  {
    grm::StoryReactOptions opts;
    std::string err;
    std::vector<std::string> args = {"--story-id=25", "--emoji=❤️", "--chat=44"};
    bool ok = grm::parse_story_react_args(args, opts, err);
    TEST_ASSERT(ok);
    TEST_ASSERT(opts.story_id == 25);
    TEST_ASSERT(opts.emoji == "❤️");
    TEST_ASSERT(opts.chat_id == 44);
  }

  {
    grm::StoryPrivacyOptions opts;
    std::string err;
    std::vector<std::string> args = {"--story-id=30", "--privacy=contacts",
                                     "--chat=66"};
    bool ok = grm::parse_story_privacy_args(args, opts, err);
    TEST_ASSERT(ok);
    TEST_ASSERT(opts.story_id == 30);
    TEST_ASSERT(opts.privacy == "contacts");
    TEST_ASSERT(opts.chat_id == 66);
  }

  std::cout << "[PASS] test_story_equals_flag_parsing\n";
}

int main() {
  test_period_parsing();
  test_story_post_arg_parsing();
  test_story_ls_and_delete_args();
  test_story_json_builders();
  test_story_info_and_viewers_args();
  test_story_pin_react_privacy_args();
  test_story_stickers_and_actions_json();
  test_story_equals_flag_parsing();
  std::cout << "All Story unit tests passed successfully!\n";
  return 0;
}
