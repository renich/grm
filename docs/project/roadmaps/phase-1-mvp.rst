================================
Phase 1 Roadmap: Core MVP Engine
================================

Phase Overview
==============

Phase 1 establishes the core C++23 native binary, CMake build architecture, TDLib integration, authentication state machine, and primary CLI commands.

Milestones & Atomic Tasks
=========================

Milestone 1.1: CMake & Build Infrastructure
-------------------------------------------

* **Task 1.1.1**: Setup modern C++23 CMake build configuration with strict warning flags (``-Werror -Wall -Wextra``).

  * *Delivers*: ``CMakeLists.txt``
  * *Unblocks*: Task 1.1.2

* **Task 1.1.2**: Write GNUmakefile build wrapper with user install targets (``make install-user``).

  * *Delivers*: ``GNUmakefile``
  * *Unblocks*: Task 1.2.1

Milestone 1.2: TDLib Client & RAII Memory Wrappers
--------------------------------------------------

* **Task 1.2.1**: Implement ``grm::JsonValue`` RAII memory wrapper over ``json-c`` (``[TECH-002]``).

  * *Fulfills*: ``[FUNC-003]``
  * *Unblocks*: Task 1.2.2

* **Task 1.2.2**: Implement ``grm::TdClient`` event loop and ``@extra`` request/promise router (``[TECH-001]``).

  * *Fulfills*: ``[FUNC-001]``, ``[FUNC-002]``
  * *Unblocks*: Task 1.3.1

Milestone 1.3: Application Controller & CLI Commands
-----------------------------------------------------

* **Task 1.3.1**: Implement ``grm login`` interactive authentication state machine (``[FUNC-001]``).

  * *Unblocks*: Task 1.3.2

* **Task 1.3.2**: Implement ``grm chat ls``, ``grm msg ls``, and ``grm send`` commands (``[FUNC-002]``, ``[FUNC-003]``, ``[FUNC-005]``).

  * *Unblocks*: Deployment

