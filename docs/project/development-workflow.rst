=========================================================
Development Workflow: Feature Addition & Bug Repair Procedures
=========================================================

Overview
========

This specification outlines the standard engineering procedures for adding new features and conducting bug repairs in **grm**. These protocols ensure maintainability, architectural decoupling, single-source help generation, static analysis, local testing, and documentation alignment across human developers and AI pair-programming agents.

Feature Addition Procedure
==========================

When introducing a new CLI subcommand, flag, or core subsystem to **grm**, follow this mandatory 9-step checklist:

1. Feature Branching
--------------------

* Always create a dedicated topic branch off `master`:

  .. code-block:: bash

     git checkout master && git pull
     git checkout -b feat/<feature-name>

2. Domain Interfaces & Decoupled Abstractions
---------------------------------------------

* Design clean C++23 interfaces and domain boundaries in `include/grm/`.
* Follow **SOLID** and **KISS**: keep abstractions minimal and consumer-driven.
* Avoid global state or direct cross-module coupling; route actions through controller handlers in `grm::App`.

3. Single-Source Command Registry & Automated Help
--------------------------------------------------

* Register the new command/subcommand specification in `CommandRegistry` (`include/grm/command_registry.hpp`):
  * Create `get_<module>_spec()` in `src/cmd_<module>.cpp`.
  * Push the spec into `CommandRegistry` in `src/command_registry.cpp`.
* **Automated Output**: The single-source spec automatically generates:
  * Terminal help (`grm <cmd> --help` and `grm --help=all`).
  * Programmatic JSON help (`grm -F json --help` and `grm -F json <cmd> --help`).
  * Introspective shell completions (`grm completion bash|zsh|fish`).

4. Documentation & Changelog Synchronization
-------------------------------------------

* Update **Changelog**: Document all user-facing additions, changes, and fixes under the `[Unreleased]` section of `CHANGELOG.rst` following Keep a Changelog 1.1.0 guidelines.
* Update **Man Page**: Document all new flags, subcommands, and usage examples in `docs/man/grm.1.rst`.
* Update **User Guide**: Add detailed subcommand tutorials in `docs/user/grm.rst`.
* Update **README**: Add a succinct, high-level entry in `README.rst` keeping descriptions generalized.
* Verify syntax with `make doc-check` (`rstcheck`).

5. Automated Unit & Integration Tests (TDD)
-------------------------------------------

* Create or update test executables in `tests/` (e.g. `tests/test_<feature>.cpp`).
* Register test targets in `CMakeLists.txt` (`add_executable`, `target_link_libraries`, `add_test`).
* Ensure all assertions pass without hardcoded mocks or tautological assertions.

6. Local Build & Live Testing
-----------------------------

* Compile and verify all CTest targets locally:

  .. code-block:: bash

     make check

* Test the build binary (`./build/grm`) against your live Telegram account to verify empirical runtime behavior.

7. User-Local Installation
--------------------------

* Install the compiled release binary, updated man page, and completions to user home (`~/.local`):

  .. code-block:: bash

     make install-user

* Verify the installed binary (`~/.local/bin/grm`) directly in your user shell environment.

8. Static Analysis & Code Formatting
-------------------------------------

* Run strict linting and formatting targets:

  .. code-block:: bash

     make format   # Apply clang-format
     make lint     # Run clang-tidy static analysis
     make analyze  # Run Clang scan-build static analyzer

9. Commit & Merge Request Pipeline
----------------------------------

* Commit using **Conventional Commits**: `feat(<scope>): description`.
* Push feature branch to remote and verify GitLab CI/CD container pipeline execution.
* Merge feature branch into `master` after pipeline passes.

Bug Repair Procedure
====================

Bug repairs must be practical, root-cause focused, and rapid while maintaining quality contracts.

1. Bugfix Branching
-------------------

* Create a dedicated bugfix branch:

  .. code-block:: bash

     git checkout master && git pull
     git checkout -b fix/<bug-description>

2. Reproduction Test
--------------------

* Write a minimal failing test in `tests/` reproducing the exact bug, stack trace, or unexpected behavior.
* Run `make check` to confirm test failure.

3. Root-Cause Fix
-----------------

* Modify implementation in `src/` or `include/` to resolve the underlying root cause.
* **Strict Directive**: Never swallow exceptions, mask symptoms with silent fallbacks, or delete broken assertions to force green tests.

4. Regression Verification
--------------------------

* Run full CTest test suite to verify the bug is resolved and no regressions were introduced:

  .. code-block:: bash

     make check

5. Documentation & Changelog Synchronization
-------------------------------------------

* Document the bug resolution under `.. rubric:: Fixed` in `CHANGELOG.rst`.
* If the bugfix altered option flags, defaults, or behavior, update `CommandRegistry`, `docs/man/grm.1.rst`, and `docs/user/grm.rst`.
* Verify documentation syntax with `make doc-check`.

6. Local User Installation & Live Verification
----------------------------------------------

* Install updated binary to user home (`make install-user`).
* Execute live verification tests against real environment to confirm resolution.

7. Commit & Merge Procedure
---------------------------

* Commit using **Conventional Commits**: `fix(<scope>): description`.
* Rebase locally against `master` and fast-forward merge into `master`.
