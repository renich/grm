==========================================================
Phase 8 — Automated Release Pipeline & FHS 3.0 Packaging
==========================================================

:Status: Completed & Deployed
:Version: v0.5.0
:Date: 2026-08-05

Executive Summary
=================

Phase 8 implements automated continuous integration, tag-triggered release pipelines, cross-platform binary releases, and standard FHS 3.0 / XDG Base Directory local user packaging for **grm**.

Deliverables & Scope
====================

1. **Tag-Triggered Release Pipeline**:
   * Multi-stage GitLab CI pipeline (``lint`` -> ``build`` -> ``test`` -> ``release``).
   * Automated release asset generation (stripped release binary ``grm``, man page ``grm.1``).
   * Idempotent tag release publishing on GitLab and GitHub with pure POSIX Markdown changelog extraction.

2. **FHS 3.0 & XDG Base Directory Compliance**:
   * Local non-root user installation target ``make install-user`` installing to ``~/.local/bin/grm``, ``~/.local/share/man/man1/grm.1``, and ``~/.local/share/bash-completion/completions/grm``.
   * System-wide installation target ``sudo make install`` installing to ``/usr/local/bin/grm``.

3. **Pre-Push Quality Automation**:
   * Local Git pre-push hook (``.githooks/pre-push.bash``) executing ``crstlint``, ``make doc-check``, ``make format``, ``make check``, and ``make lint`` prior to remote pushing.
   * Atomic branch and tag push workflow.
