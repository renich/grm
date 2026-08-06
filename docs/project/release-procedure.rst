===============================================
Release Procedure & Standard Operating Protocol
===============================================

This document defines the official step-by-step procedure for releasing new versions of **grm** (Group & Telegram Manager CLI).

.. contents:: Table of Contents
   :depth: 2
   :local:

Overview & Release Cadence
==========================

* Releases strictly adhere to `Semantic Versioning 2.0.0 <https://semver.org/spec/v2.0.0.html>`_ (``MAJOR.MINOR.PATCH``).
* Every release must pass unit tests, integration tests, static documentation checks, and clean local builds prior to tagging.
* All changes must be fully documented in ``CHANGELOG.rst`` under a dedicated release section **before** creating a Git release tag.

Pre-Release Checklist
=====================

Before initiating a release, complete the following verification steps:

1. **Run Local Test Suite**:
   Verify that all CTest test cases pass without errors:

   .. code-block:: bash

      cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
      cmake --build build
      ctest --test-dir build --output-on-failure

2. **Run Documentation Validation**:
   Validate reStructuredText syntax for man pages and user documentation:

   .. code-block:: bash

      make doc-check

3. **Verify Clean Working Tree**:
   Ensure no untracked files or unstaged changes remain in the repository:

   .. code-block:: bash

      git status

Release Execution Protocol
==========================

Follow these steps in sequence to perform a clean release:

Step 1: Update CHANGELOG.rst
----------------------------

1. Open ``CHANGELOG.rst``.
2. Move all documented items from under ``[Unreleased]`` into a new version header matching the release tag and current date:

   .. code-block:: rst

      [0.5.2] — 2026-08-06
      ====================

      .. rubric:: Added

      * ...

      .. rubric:: Changed

      * ...

      .. rubric:: Fixed

      * ...

3. Preserve an empty ``[Unreleased]`` header at the top of ``CHANGELOG.rst`` for future contributions.

Step 2: Commit the Release Changelog
------------------------------------

Commit the updated ``CHANGELOG.rst`` file using Conventional Commits syntax:

.. code-block:: bash

   git commit -am "release(v0.5.2): update CHANGELOG.rst for v0.5.2 release"

Step 3: Create Annotated Release Tag
-------------------------------------

Create an annotated Git tag matching the release version:

.. code-block:: bash

   git tag -a v0.5.2 -m "Release v0.5.2 - Brief summary of release features"

Step 4: Push Commits and Tags to Remotes
----------------------------------------

Push both the updated ``master`` branch and the release tag to all project remotes (GitLab ``origin`` and GitHub ``github``):

.. code-block:: bash

   git push origin master v0.5.2
   git push github master v0.5.2

Step 5: Verify Automated CI/CD Release Pipeline
------------------------------------------------

1. Monitor the GitLab CI tag pipeline:

   .. code-block:: bash

      glab pipeline list --repo renich/grm

2. Verify that all stages (``lint``, ``build``, ``test``, and ``release``) pass cleanly.
3. Confirm that the GitLab Release page automatically generated release notes from ``CHANGELOG.rst``.
