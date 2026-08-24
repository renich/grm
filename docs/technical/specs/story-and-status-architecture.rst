============================================
Story & Status Subsystem Architecture Spec
============================================

:Author: Rénich Bon Ćirić
:Status: Approved
:Type: Technical Architecture Specification
:Date: 2026-08-23

Architectural Overview
======================

The **grm** Story and Custom Emoji Status subsystem provides stateless C++23 CLI endpoints for TDLib's Story and Status APIs, maintaining full backward and forward compatibility across local builds and system package updates.

Component Architecture
======================

1. Module Layout
----------------

* ``include/grm/cmd_story.hpp`` / ``src/cmd_story.cpp``: Story argument parsing, payload builders, and execution handlers.
* ``include/grm/cmd_status.hpp`` / ``src/cmd_status.cpp``: Emoji status parsing, duration calculation, and execution handlers.
* ``include/grm/command_registry.hpp`` / ``src/command_registry.cpp``: Self-registering subcommand metadata for global help and autocompletions.

2. TDLib JSON Dispatching
-------------------------

* **Story Posting**: Uses TDLib ``postStory`` with ``inputStoryContentPhoto`` or ``inputStoryContentVideo`` and nested ``storyPrivacySettings``.
* **Story Listing**: Dispatches ``getChatActiveStories``.
* **Story Deletion**: Dispatches ``deleteStory``.
* **Emoji Status Setting**: Dispatches ``setEmojiStatus`` (for user profiles) or ``setChatEmojiStatus`` (for supergroups/channels) with ``emojiStatusTypeCustomEmoji``.
* **Emoji Status Clearing**: Dispatches ``setEmojiStatus`` / ``setChatEmojiStatus`` with ``null`` status payload.

3. Local TDLib Dependency Hierarchy & Fallback
----------------------------------------------

CMake discovery prioritizes ``~/.local/lib64/pkgconfig`` and ``~/.local/lib/pkgconfig`` before system paths. When Fedora packages update to ``tdlib-devel >= 1.8.20``, removal of the local prefix triggers an immediate, seamless fallback to the system library without build system reconfiguration.
