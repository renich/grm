===============================================================
Phase 5 Roadmap: Modern Dual Human/AI UX Render Engine (C++23)
===============================================================

Overview
--------

Phase 5 replaces legacy log-level output with a domain-driven, polymorphic C++23 Visitor Render Engine (``Formatter``) supporting Human, Markdown, JSON Envelopes, and NDJSON streaming.

Milestones & Tasks
------------------

* **M5.1: Domain Data Models & Formatting Primitives** *(Completed)*

  - Created ``ChatItem``, ``TopicItem``, ``MessageItem``, and ``ErrorPayload`` models.
  - Implemented ISO-8601 UTC date formatting, humanized bytes, and relative timestamps.

* **M5.2: Polymorphic Render Engine Dispatcher** *(Completed)*

  - Implemented ``Formatter::render()`` visitor over ``RenderablePayload`` variant.
  - Supported ``OutputFormat::Json`` (Envelope) and ``OutputFormat::JsonL`` (NDJSON).

* **M5.3: Domain Provider Integration** *(Completed)*

  - Extracted domain attributes across ``cmd_chat_ls``, ``cmd_topic_ls``, ``cmd_msg_ls``, and ``cmd_msg_search``.

* **M5.4: CLI Option Integration & Shell Completion** *(Completed)*

  - Added ``-F, --format <human|markdown|json|plain>`` and ``--color <auto|always|never>``.
  - Integrated ``Formatter`` across ``cmd_chat_ls``, ``cmd_topic_ls``, ``cmd_msg_ls``, and ``cmd_msg_search``.
  - Updated ``completions/grm.bash`` completion script.

Verification & Quality Standards
--------------------------------

- All formatting methods covered in ``tests/test_render_engine.cpp``.
- 100% CTest pass rate across all 13 test executables.
