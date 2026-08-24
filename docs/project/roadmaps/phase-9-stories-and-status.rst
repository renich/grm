=============================================
Phase 9: Telegram Stories & Emoji Status CLI
=============================================

:Author: Rénich Bon Ćirić
:Status: Completed
:Type: Project Roadmap Specification
:Date: 2026-08-23

Overview
========

Phase 9 integrates full Telegram Story and Custom Emoji Status capabilities into **grm**, backed by an updated TDLib (>= 1.8.20) build and seamless system fallback architecture.

Delivered Scope
===============

1. **Build & Fallback Architecture**:

   * Prepend ``$HOME/.local`` to CMake prefix and PKG_CONFIG_PATH search hierarchies.
   * Automated ``make tdlib-bootstrap`` target to build upstream TDLib into ``$HOME/.local``.
   * Zero-config automatic fallback to system package when Fedora updates ``tdlib-devel``.

2. **Story Subcommand Suite** (``grm story``):

   * ``grm story post``: Photo/video story publishing with Markdown captions, privacy settings (``everyone``, ``contacts``, ``close_friends``), duration periods (6h, 12h, 24h, 48h), profile pinning, and content protection.
   * ``grm story ls``: Active story listing with tabular summary and JSON envelope modes.
   * ``grm story delete``: Targeted story removal.

3. **Custom Emoji Status Subcommand Suite** (``grm status``):

   * ``grm status set``: Custom emoji badge assignment with human duration parsing (e.g. ``30m``, ``1h``, ``2d``).
   * ``grm status clear``: Badge removal.
   * Chat-scoped status support for boosted supergroups and channels (``--chat <id>``).

4. **Testing & Tooling**:

   * Unit test suites: ``tests/test_story.cpp`` and ``tests/test_status.cpp``.
   * 100% test pass rate across all 20 CTest suites under AddressSanitizer, UBSan, and LeakSanitizer.
   * Bash completion and man page documentation updates.
