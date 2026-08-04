==========================================================
Phase 3 Roadmap: File Uploads & Supergroup Forum Topics
==========================================================

Phase Overview
==============

Phase 3 introduces local file & media attachment uploads and Supergroup Forum Topics discovery & message thread targeting to **grm**.

Milestones & Atomic Tasks
=========================

Milestone 3.1: File & Media Attachment Upload Engine
----------------------------------------------------


* **Task 3.1.1**: Implement ``inputFileLocal`` & ``inputMessageDocument`` payload generator in ``src/cmd_file.cpp`` and unit test in ``tests/test_file.cpp``.
  * *Delivers*: ``src/cmd_file.cpp``, ``tests/test_file.cpp``
  * *Fulfills*: ``[FUNC-007]``, ``[TECH-004]``
  * *Status*: ``[ ]`` Pending
  * *Unblocks*: Task 3.1.2

* **Task 3.1.2**: Implement ``grm send file <chat_id> <file_path> [--caption "text"] [--topic <topic_id>]`` CLI command.
  * *Delivers*: ``src/cmd_file.cpp``
  * *Fulfills*: ``[FUNC-007]``
  * *Status*: ``[ ]`` Pending
  * *Unblocks*: Task 3.2.1

Milestone 3.2: Forum Topics Discovery & Thread-Scoped Messaging
---------------------------------------------------------------

* **Task 3.2.1**: Implement ``grm topic ls <supergroup_id>`` forum topics discovery command via TDLib ``getForumTopics``.
  * *Delivers*: ``src/cmd_topic.cpp``
  * *Fulfills*: ``[FUNC-008]``, ``[TECH-005]``
  * *Status*: ``[ ]`` Pending
  * *Unblocks*: Task 3.2.2

* **Task 3.2.2**: Extend ``grm send``, ``grm msg ls``, and ``grm send file`` with ``--topic <topic_id>`` (``message_thread_id``) support.
  * *Delivers*: ``src/cmd_msg.cpp``, ``src/cmd_file.cpp``
  * *Fulfills*: ``[FUNC-008]``
  * *Status*: ``[ ]`` Pending
  * *Unblocks*: Phase 3 Completion
