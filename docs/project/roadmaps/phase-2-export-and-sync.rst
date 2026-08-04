===========================================================
Phase 2 Roadmap: Unit Testing, Modularization & Data Export
===========================================================

Phase Overview
==============

Phase 2 introduces automated test infrastructure, refactors monolithic source modules to adhere strictly to code size rules (<300 lines), and implements message history data export (CSV/JSON) and regex search filtering.

Milestones & Atomic Tasks
=========================

Milestone 2.1: Test Infrastructure & App Modularization
-------------------------------------------------------

* **Task 2.1.1**: Setup CTest and implement unit tests for ``grm::JsonValue``, ``grm::Config``, and birthday extraction regex matching.
  * *Delivers*: ``tests/test_json_utils.cpp``, ``tests/test_config.cpp``, ``tests/test_extract.cpp``
  * *Status*: ``[~]`` In Progress
  * *Unblocks*: Task 2.1.2

* **Task 2.1.2**: Refactor monolithic ``src/app.cpp`` (>300 lines) into modular command handlers.
  * *Delivers*: ``src/cmd_auth.cpp``, ``src/cmd_chat.cpp``, ``src/cmd_msg.cpp``, ``src/cmd_extract.cpp``
  * *Status*: ``[ ]`` Pending
  * *Unblocks*: Task 2.2.1

Milestone 2.2: Data Export & Search Filtering Engine
----------------------------------------------------

* **Task 2.2.1**: Implement ``grm msg export <chat_id> csv|json [filename]`` command for exporting chat history into structured CSV and JSON formats.
  * *Delivers*: ``src/cmd_export.cpp``, ``include/grm/exporter.hpp``, ``src/exporter.cpp``
  * *Status*: ``[ ]`` Pending
  * *Unblocks*: Task 2.2.2

* **Task 2.2.2**: Implement ``grm msg search <chat_id> "<query>"`` regex search filtering CLI command.
  * *Delivers*: ``src/cmd_search.cpp``
  * *Status*: ``[ ]`` Pending
  * *Unblocks*: Phase 2 Completion
