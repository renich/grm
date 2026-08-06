====================
Changelog — grm
====================

All notable changes to the **grm** (Group & Telegram Manager CLI) project will be documented in this file.

The format is based on `Keep a Changelog 1.1.0 <https://keepachangelog.com/en/1.1.0/>`_, and this project adheres to `Semantic Versioning <https://semver.org/spec/v2.0.0.html>`_.

[Unreleased]
============

.. rubric:: Added

* Sender name format configuration (``sender_name_format``: ``username`` | ``fullname``) defaulting to ``username`` (with fallback to display name).
* CLI flags ``-N | --name-format <username|fullname>``, ``-u | --username``, and ``--full-name`` for name format selection.
* Message date range filtering via ``-S | --since <duration|date>`` flag in ``grm msg ls`` and ``grm msg search`` (supporting relative durations like ``1d``, ``2h``, ISO dates, and Unix timestamps).
* Automatic sender name resolution for message list and search commands across all output formats.
* Dynamic RPATH configuration linking against local ``libtdjson.so.1.8.66`` in ``~/.local/lib``.

.. rubric:: Changed

* Upgraded TDLib compatibility layer to support TDLib 1.8.66 / MTProto Layer 228 API specifications.
* Gated ``[MsgID <id>]`` display prefix in message list output to verbose (``-v`` / ``--verbose``) mode.
* Flattened ``setTdlibParameters`` request payload structure for modern TDLib release standards.

[0.5.0] — 2026-08-05
====================

.. rubric:: Added

* Modern C++23 Dual Human/AI UX Render Engine with polymorphic ``std::variant`` payload dispatch.
* Support for ``OutputFormat::Json`` (JSON Envelopes with ``status``, ``count``, and ISO-8601 timestamps) and ``OutputFormat::JsonL`` (NDJSON line-delimited streaming).
* Environment variable fallback support for ``GRM_FORMAT`` (``json``, ``jsonl``, ``human``, ``markdown``, ``plain``), ``GRM_COLOR``, and ``NO_COLOR``.
* Unified ``grm file get [-A|--all]`` command supporting both specific message attachment downloads and bulk chat/topic media downloads.
* Comprehensive CTest unit and integration test suite with 13 test executables.
* Official GNU General Public License v3.0 (``LICENSE``) and contributor guidelines (``CONTRIBUTING.rst``).

.. rubric:: Changed

* Refactored CLI subcommand routing in ``src/app.cpp`` to dispatch dedicated help screens with exit code 0 when invoked without subcommands.
* Consolidated file download subcommands under ``grm file get`` while retaining ``grm file download-all`` as a seamless alias.
* Upgraded ``GNUmakefile`` to strict GNU Coding Standards with automatic dependency generation, ``uninstall`` targets, and formatted ``help`` output.
* Redesigned project emblem logo in Crystal Flaw Shard geometric gem style (512x512 PNG/SVG).

.. rubric:: Fixed

* Fixed TDLib 1.8.66 schema wrappers for document (``inputDocument``) and photo (``inputPhoto``) file uploads.
* Fixed JSON serialization key ordering ensuring ``@type`` and ``@extra`` appear first in TDLib request payloads.
* Fixed CLI subcommand help screen crash when executing ``grm chat --help`` or ``grm chat``.

[0.4.0] — 2026-08-04
====================

.. rubric:: Added

* Complete Supergroup Forum Topic CRUD management (``grm topic ls``, ``create``, ``info``, ``edit``, ``close``, ``reopen``, ``pin``, ``unpin``, ``delete``).
* Forum topic custom emoji icon customization support via ``-e | --emoji`` and ``--icon-color``.

[0.3.0] — 2026-08-03
====================

.. rubric:: Added

* Telegram Rich Text Markdown V2 formatting integration via TDLib ``parseTextEntities``.
* Message history search engine with regex query filtering (``grm msg search``).
* Chat history export to CSV and JSON formats (``grm msg export``).
* Document and compressed photo attachment upload engine (``grm msg send -a``).

[0.2.0] — 2026-08-02
====================

.. rubric:: Added

* Chat and group lifecycle management (``grm chat ls``, ``create group/channel``, ``info``, ``set-title``, ``set-desc``, ``pin``, ``unpin``, ``delete``).
* Structured log levels (``[INFO]``, ``[AUTH]``, ``[DEBUG]``, ``[WARN]``, ``[ERROR]``).

[0.1.0] — 2026-08-01
====================

.. rubric:: Added

* Initial project skeleton with C++23 CMake build system.
* Asynchronous TDLib client wrapper (``TdClient``) and RAII JSON parser (``JsonValue``).
* Interactive and non-interactive authentication flow (``grm login``).
