======================================================
[TECH-010] Release Procedure & Versioning Playbook
======================================================

Overview
--------
This document defines the deterministic, repeatable release workflow for ``grm``. It enforces Semantic Versioning (SemVer 2.0.0), Keep a Changelog 1.1.0 compliance, multi-file version alignment, local pre-push verification, signed GPG Git tags, and automated GitLab CI release artifact creation.

Requirement Mapping
-------------------
Fulfills `[FUNC-001]`.

Multi-File Version Alignment Checklist
---------------------------------------
Prior to tagging any release, the version string (e.g. ``0.8.1``) must be aligned across the codebase:

#. **`CMakeLists.txt <CMakeLists.txt>`_**:

   .. code-block:: cmake

      project(grm VERSION 0.8.1 LANGUAGES CXX)

#. **`packaging/grm.spec <packaging/grm.spec>`_**:

   .. code-block:: spec

      Version:        0.8.1

#. **`docs/conf.py <docs/conf.py>`_**:

   .. code-block:: python

      release = "0.8.1"

#. **`CHANGELOG.rst <CHANGELOG.rst>`_**:
   Promote items from ``[Unreleased]`` under a new version header using ``.. rubric::`` sections:

   .. code-block:: rst

      [0.8.1] - 2026-08-24
      ====================

      .. rubric:: Added

      - Feature additions.

Step-by-Step Release Execution Playbook
---------------------------------------

Step 1: Local Quality Assurance & Verification
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Execute the full test suite and quality gates locally:

.. code-block:: bash

   make check
   make asan
   make tsan

Step 2: Commit Version Bump & Changelog Update
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Stage and commit version changes adhering to Conventional Commit standards:

.. code-block:: bash

   git add CMakeLists.txt docs/conf.py CHANGELOG.rst packaging/
   git commit -m "chore(release): bump version to 0.8.1"

Step 3: Create Signed Annotated Git Tag
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Create a GPG-signed Git tag for the release:

.. code-block:: bash

   git tag -s v0.8.1 -m "Release v0.8.1"

Step 4: Push Branch and Tags to GitLab
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Push the release commit and tag to the remote GitLab repository:

.. code-block:: bash

   git push origin master --tags
