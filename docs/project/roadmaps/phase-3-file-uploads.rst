================================================
Phase 3 Roadmap: File & Media Attachment Uploads
================================================

Phase Overview
==============

Phase 3 introduces local file & media attachment uploads to **grm**.

Milestones & Atomic Tasks
=========================

Milestone 3.1: File & Media Attachment Upload Engine
----------------------------------------------------

* **Task 3.1.1**: Implement ``inputFileLocal`` & ``inputMessageDocument`` payload generator in ``src/cmd_file.cpp`` and unit test in ``tests/test_file.cpp``.

  * *Delivers*: ``src/cmd_file.cpp``, ``include/grm/uploader.hpp``, ``src/uploader.cpp``, ``tests/test_file.cpp``
  * *Fulfills*: ``[FUNC-007]``, ``[TECH-004]``
  * *Status*: ``[x]`` Completed
  * *Unblocks*: Task 3.1.2

* **Task 3.1.2**: Implement ``grm send file <chat_id> <file_path> [--caption "text"]`` CLI command.

  * *Delivers*: ``src/cmd_file.cpp``
  * *Fulfills*: ``[FUNC-007]``
  * *Status*: ``[x]`` Completed
  * *Unblocks*: Phase 3 Completion

