========================
Contributing to grm
========================

Thank you for your interest in contributing to **grm** (Group & Telegram Manager)! We welcome contributions from developers, sysadmins, and AI pair-programming systems.

This document outlines the engineering standards, development workflows, testing requirements, and contribution guidelines for the project.

Core Engineering Precepts
=========================

* **KISS (Keep It Simple, Stupid)**: Write clear, direct C++23 code. Avoid premature abstractions and over-engineering.
* **DRY (Don't Repeat Yourself)**: Consolidate duplicated logic into reusable functions or domain helpers.
* **SOLID**: Enforce single responsibility boundaries, composition over inheritance, and consumer-side interfaces.
* **Boy Scout Rule**: Leave any code or documentation cleaner than you found it.
* **Dual Human/AI UX**: Ensure CLI output supports both rich ANSI terminal interaction and structured machine parsing (JSON/NDJSON).

Getting Started
===============

1. **Fork & Clone**:
   Fork the primary repository on `GitLab <https://gitlab.com/renich/grm>`_ (or mirror on `GitHub <https://github.com/renich/grm>`_) and clone locally:

   .. code-block:: bash

      git clone https://gitlab.com/renich/grm.git
      cd grm

2. **Install Build Dependencies (Fedora Linux)**:

   .. code-block:: bash

      sudo dnf install tdlib-devel json-c-devel cmake ninja-build gcc-c++ clang-tools-extra rstcheck python3-docutils

3. **Build & Verify Test Suite**:

   .. code-block:: bash

      make check

Development Workflow & Engineering Procedures
================================================

For complete step-by-step procedures, refer to the authoritative specification: `docs/project/development-workflow.rst <docs/project/development-workflow.rst>`_.

Feature Addition Procedure (9-Step Protocol)
--------------------------------------------

1. **Branching**: Create topic branch `git checkout -b feat/<feature-name>`.
2. **Domain Interfaces**: Define clean C++23 interfaces in `include/grm/` (SOLID/KISS).
3. **Single-Source Help**: Register `get_<module>_spec()` in `CommandRegistry` (`src/cmd_<module>.cpp` & `src/command_registry.cpp`) to auto-generate `--help`, `-F json --help`, and shell completions (`grm completion bash`).
4. **Documentation Sync**: Update man page (`docs/man/grm.1.rst`), user guide (`docs/user/grm.rst`), and `README.rst` (succinct & general). Verify syntax with `make doc-check`.
5. **Automated Testing**: Add TDD test suite in `tests/` and register target in `CMakeLists.txt`.
6. **Local Build & Test**: Run `make check` (100% CTest pass) and test `./build/grm` with live account.
7. **Local User Installation**: Run `make install-user` (PREFIX `~/.local`) and verify `~/.local/bin/grm`.
8. **Static Analysis**: Run `make format` (`clang-format`), `make lint` (`clang-tidy`), and `make analyze`.
9. **Commit & MR**: Conventional Commit (`feat(...)`), push, verify GitLab CI, and merge.

Bug Repair Procedure (Practical 7-Step Protocol)
------------------------------------------------

1. **Branching**: Create fix branch `git checkout -b fix/<bug-name>`.
2. **Reproduction**: Write a minimal failing test in `tests/` reproducing the bug.
3. **Root-Cause Fix**: Resolve the underlying defect in `src/` (no symptom masking or swallowed errors).
4. **Regression Check**: Run `make check` to ensure 100% test pass rate.
5. **Help/Doc Sync**: Update `CommandRegistry` or man page if flag behavior was modified.
6. **Local Install**: Run `make install-user` and verify live behavior.
7. **Commit & MR**: Conventional Commit (`fix(...)`), push, and merge.

Test-Driven Development (TDD)
-----------------------------

All feature additions, refactorings, and bug fixes **must** be accompanied by unit or integration tests:

1. **Write failing test(s)** in `tests/` reproducing the bug or asserting new capability.
2. **Implement minimal code** in `src/` to satisfy the tests.
3. **Refactor** while ensuring all CTest targets continue to pass 100%.

Git Pre-Push Hook & Quality Automation
---------------------------------------

`grm` provides a version-controlled Git pre-push hook (`.githooks/pre-push.bash`) that allows local work-in-progress (`WiP`) commits while guaranteeing remote code quality, document syntax, formatting, and test pass rates prior to pushing upstream.

To configure the pre-push hook in your clone:

.. code-block:: bash

   make install-hooks

Running Quality Checks
----------------------

Before pushing or submitting a Merge Request (MR) or Pull Request (PR), ensure all quality suites pass:

.. code-block:: bash

   # Run CTest automated test suite
   make check

   # Apply clang-format code formatting
   make format

   # Run clang-tidy static analysis
   make lint

   # Compile man page and verify reStructuredText syntax
   make man
   make doc-check

   # Run crstlint reStructuredText linter across all RST documents
   crstlint -fr .

Commit Message Standards
========================

`grm` strictly enforces the **Conventional Commits** specification:

* **Format**: ``type(scope): concise description in imperative mood``
* **Allowed Types**:

  * ``feat``: New CLI command, flag, or output capability.
  * ``fix``: Bug fix or crash resolution.
  * ``docs``: Documentation, man page, or spec updates.
  * ``test``: Unit or integration test additions.
  * ``refactor``: Code refactoring without behavioral change.
  * ``chore``: Build configuration or dependency updates.

**Example Commit Message**:

.. code-block:: text

   feat(render): add NDJSON streaming output mode for msg ls

   - Added OutputFormat::JsonL payload renderer in src/formatter.cpp.
   - Updated main CLI option parser to register -F jsonl and -F ndjson.
   - Added unit test cases to tests/test_render_engine.cpp.

Documentation Standards
=======================

* **Format**: All user documentation, manual pages, and architectural specs must be written in **reStructuredText** (`.rst`).
* **Indentation**: Use **3-space indentation** for RST directives.
* **Rubrics**: Use `.. rubric::` for minor section headings inside logs or sub-documents to prevent Sphinx/rstcheck outline warnings.
* **Man Page**: Keep `docs/man/grm.1.rst` aligned with any CLI option or subcommand changes.

Submitting Merge Requests
=========================

1. Push your topic branch to your fork.
2. Open a Merge Request against `GitLab master <https://gitlab.com/renich/grm/-/merge_requests/new>`_.
3. Ensure CI/CD pipelines pass all builds, linting checks, and CTest test suites.
