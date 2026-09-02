============================================
Story & Status Subsystem Architecture Spec
============================================

:Author: Rénich Bon Ćirić
:Status: Approved
:Type: Technical Architecture Specification
:Date: 2026-08-24

Architectural Overview
======================

The **grm** Story and Custom Emoji Status subsystem provides stateless C++23 CLI endpoints for TDLib's Story and Status APIs, maintaining full backward and forward compatibility across local builds and system package updates.

Component Architecture
======================

1. Module Layout
----------------

* ``include/grm/cmd_story.hpp`` / ``src/cmd_story.cpp``: Story argument parsing, interactive sticker payload builders, metadata prefetching, and execution handlers.
* ``include/grm/cmd_status.hpp`` / ``src/cmd_status.cpp``: Emoji status parsing, duration calculation, and execution handlers.
* ``include/grm/json_utils.hpp`` / ``src/json_utils.cpp``: RAII JSON utility wrappers including ``JsonValue::get_double`` and ``JsonValue::as_double``.
* ``include/grm/command_registry.hpp`` / ``src/command_registry.cpp``: Self-registering subcommand metadata for global help and autocompletions.

2. TDLib JSON Dispatching & Preload Protocols
---------------------------------------------

* **Story Posting & Overlays**: Dispatches ``postStory`` with ``inputStoryContentPhoto`` or ``inputStoryContentVideo``, ``storyPrivacySettings`` (``everyone``, ``contacts``, ``close_friends``), and interactive sticker areas via ``inputStoryAreas``:

  * Clickable link sticker: ``inputStoryAreaTypeLink`` with normalized ``storyAreaPosition`` coordinates.
  * Suggested reaction sticker: ``inputStoryAreaTypeSuggestedReaction`` with ``reactionTypeEmoji``.
  * Upload tracking: Listens for ``updateStoryPostSucceeded`` (and legacy ``updateStorySendSucceeded``) matching ``old_story_id`` against the provisional story ID (>= 2,000,000,000) to confirm upload and obtain the finalized server story ID.

* **Story Listing**: Dispatches ``getChatActiveStories``, ``getChatPostedToChatPageStories``, and ``getChatArchivedStories``, fetching detailed story metadata via ``getStory`` with human-readable timestamps and pinned/posted indicators.
* **Story Metadata Inspection**: Dispatches ``getStory`` and formats comprehensive metadata (published date, sender ID, media type, view count, forward count, reaction count, capabilities, status, and interactive stickers).
* **Viewers & Analytics**: Dispatches ``getStoryInteractions`` with query filtering and pagination limits to list viewers, interaction types, and reaction emojis.
* **Pin & Unpin Management**: Dispatches ``toggleStoryIsPostedToChatPage`` after preloading the story into TDLib memory via ``getStory``.
* **Story Reactions**: Dispatches ``setStoryReaction`` with ``reactionTypeEmoji`` or ``null`` to remove reaction, preloaded via ``getStory``.
* **Privacy Updates**: Dispatches ``setStoryPrivacySettings`` with updated ``storyPrivacySettings`` payload.
* **Stealth Mode**: Dispatches ``activateStoryStealthMode`` for 25-minute anonymous story viewing.
* **Story Editing**: Uses TDLib ``editStory`` with ``inputStoryContentPhoto`` or ``inputStoryContentVideo``, updated ``inputStoryAreas``, and ``formattedText`` entities for captions.
* **Story Deletion**: Dispatches ``deleteStory`` with ``story_poster_chat_id``.
* **Emoji Status Discovery**: Dispatches ``getDefaultEmojiStatuses``, ``getRecentEmojiStatuses``, and ``getInstalledStickerSets`` (incorporating official sticker packs such as ``Animated Emoji``, ``Emoticon Emoji``, and ``Kawaii Emoji``). Custom emojis in Telegram are vector document objects (TGS Lottie, WebM VP9, or WebP) identified by 64-bit ``document_id`` (TDLib ``custom_emoji_id``) with a Unicode fallback ``alt`` string. Document details are resolved in chunks of 50 via ``getCustomEmojiStickers`` (extracting ``custom_emoji_id`` from ``stickerFullTypeCustomEmoji``) and pack titles via ``getStickerSet``.
* **Emoji Status Setting**: Dispatches ``setEmojiStatus`` (for user profiles) or ``setChatEmojiStatus`` (for supergroups/channels) with ``emojiStatusTypeCustomEmoji``.
* **Emoji Status Clearing**: Dispatches ``setEmojiStatus`` / ``setChatEmojiStatus`` with ``null`` status payload.

3. Local TDLib Dependency Hierarchy & Fallback
----------------------------------------------

CMake discovery prioritizes ``~/.local/lib64/pkgconfig`` and ``~/.local/lib/pkgconfig`` before system paths. When Fedora packages update to ``tdlib-devel >= 1.8.20``, removal of the local prefix triggers an immediate, seamless fallback to the system library without build system reconfiguration.
