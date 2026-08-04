===============================================================
Phase 5 Roadmap: Human & LLM Output Formatting Engine
===============================================================

Overview
--------

Phase 5 introduces a unified, modular formatting subsystem (**grm::fmt**) designed for both human terminal interaction and structured AI/LLM data ingestion.

Milestones & Deliverables
-------------------------

* **M5.1: Specification & Design** *(Completed)*
  - Authored functional specification ``docs/functional/specs/output-formatting-and-llm-modes.rst``.
  - Authored technical architecture ``docs/technical/specs/formatter-architecture.rst``.

* **M5.2: Test-Driven Unit Test Suite (TDD)** *(Completed)*
  - Created ``tests/test_formatter.cpp`` testing ANSI string styling, type humanization dictionaries, and Markdown table output.

* **M5.3: Core Formatter Domain Subsystem** *(Completed)*
  - Implemented ``include/grm/formatter.hpp`` and ``src/formatter.cpp``.
  - Implemented humanization mappings (e.g. ``chatTypeSupergroup`` → ``Supergroup``).
  - Implemented TTY auto-detection and ``NO_COLOR`` environment checks.

* **M5.4: CLI Option Integration & Shell Completion** *(Completed)*
  - Added ``-F, --format <human|markdown|json|plain>`` and ``--color <auto|always|never>``.
  - Integrated ``grm::fmt::Formatter`` across ``cmd_chat_ls``, ``cmd_topic_ls``, ``cmd_msg_ls``, and ``cmd_msg_search``.
  - Updated ``completions/grm.bash`` completion script.

Verification & Quality Standards
--------------------------------

- Unit test suite ``test_formatter`` passes 100%.
- ``make check``, ``make lint`` (0 clang-tidy warnings), and ``make analyze`` (0 scan-build bugs) pass cleanly.
