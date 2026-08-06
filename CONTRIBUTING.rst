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

Development Workflow
====================

Test-Driven Development (TDD)
-----------------------------

All feature additions, refactorings, and bug fixes **must** be accompanied by unit or integration tests:

1. **Write failing test(s)** in `tests/` reproducing the bug or asserting new capability.
2. **Implement minimal code** in `src/` to satisfy the tests.
3. **Refactor** while ensuring all CTest targets continue to pass 100%.

Running Quality Checks
----------------------

Before submitting a Merge Request (MR) or Pull Request (PR), ensure all quality suites pass:

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
