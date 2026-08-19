====================
Changelog — grm
====================

All notable changes to the **grm** (Group & Telegram Manager CLI) project will be documented in this file.

The format is based on `Keep a Changelog 1.1.0 <https://keepachangelog.com/en/1.1.0/>`_, and this project adheres to `Semantic Versioning <https://semver.org/spec/v2.0.0.html>`_.

[Unreleased]
============

[0.7.1] — 2026-08-19
====================

.. rubric:: Added

* Added ``grm logout`` command to cleanly terminate the active TDLib session, purge local credentials, and exit with an immediate non-blocking status check if already logged out.
* Added ``-q`` / ``--qr`` flag to ``grm login`` for desktop QR code authentication via standalone Bézier-curved vector SVG (``/tmp/grm-login-qr.svg``) opened using FreeDesktop ``xdg-open``.
* Added automatic desktop fallback from phone-number login to QR code authentication when Telegram returns ``406 UPDATE_APP_TO_LOGIN``, opening ``/tmp/grm-login-qr.svg`` via ``xdg-open`` without terminal ASCII clutter on graphical desktop sessions.
* Streamlined headless environment handling by generating ``/tmp/grm-login-qr.svg`` with clear, concise instructions to download or open the file in an image viewer or browser.
* Added LeakSanitizer suppressions file (``sanitizers/lsan.supp``) for third-party ``libtdjson`` library memory allocations during AddressSanitizer test runs.
* Added Bash autocompletion, global and JSON help screens, and comprehensive manual page documentation for ``grm logout`` and ``grm login --qr``.

.. rubric:: Fixed

* Resolved Telegram in-app mobile QR code camera scanner rejection by rendering high-resolution vector geometry with curved corner markers and rounded dot modules via ``qr-code-styling``.
* Resolved TDLib session race condition (``TDLib Error [400]: Initialization parameters are needed``) on ``grm logout`` by ensuring ``ensure_authenticated()`` initializes TDLib parameters before dispatching ``logOut``.
* Replaced obsolete ``getForumTopicHistory`` TDLib request with ``getMessageThreadHistory`` in message history retrieval and file attachment downloaders.
* Fixed ``TdClient::stop()`` shutdown sequence to dispatch ``close`` before joining receiver threads, preventing unhandled teardown crashes.
* Resolved Makefile build configuration leak by explicitly enforcing ``-DENABLE_ASAN=OFF`` and ``-DENABLE_TSAN=OFF`` on user release builds.

[0.7.0] — 2026-08-07
====================

.. rubric:: Added

* Added ``-r`` / ``--reply-to <message_id>`` flag to ``grm msg send`` to support direct inline message replies in supergroups, channels, and forum topic threads (GitLab Issue #2).
* Universal Cross-Domain Search CLI suite (``grm search``, ``grm search chats``, ``supergroups``, ``channels``, ``users``, ``msgs``, ``files``) supporting keyword search, query stemming, offset pagination (``-o`` / ``--offset``), limit controls (``-n`` / ``--limit``), and chat-scoped filtering (``--chat <id>``).
* Public Handle Extraction in candidate chat resolution: automatically parses ``@username`` and ``t.me/`` links from global message text to populate directory results for generic terms like ``movie``, ``crypto``, and ``linux``.
* Deep Alphabet Suffix Probing for public channel and user discovery.
* Thread-safe ``json_c_get`` and ``json_c_put`` helpers in ``include/grm/json_utils.hpp`` wrapping json-c reference counting in a global mutex to guarantee thread-safe execution across background threads.
* Terminal Echo Protection via RAII ``TermiosGuard`` struct in ``src/cmd_auth.cpp``, ensuring terminal ECHO flags are guaranteed to be restored on early returns or exceptions.

.. rubric:: Changed

* Updated ``main.cpp`` flag forwarding logic to preserve short option flags (such as ``-p``) for subcommands without swallowing.
* Improved ``TdClient::stop()`` shutdown sequence to prevent ``td_receive`` calls on closed client instances.
* Replaced direct throwing ``std::stoll`` in ``parse_search_args`` with non-throwing ``parse_int64`` (``std::from_chars``) to prevent crash hazards on invalid CLI arguments.
* Reduced fallback chat folder probe loop timeout in ``cmd_folder_ls`` from 1.5s to 0.1s over top 10 folder IDs to eliminate blocking delays.

.. rubric:: Fixed

* Resolved premature client session teardown in ``grm msg send`` (GitLab Issue #1) by implementing ``App::send_message_and_wait`` to wait for TDLib ``updateMessageSendSucceeded`` / ``updateMessageSendFailed`` before exiting.
* Resolved supergroup/channel resolution error (``TDLib Error [400]: Chat not found``) in ``grm msg edit`` (GitLab Issue #3) by enhancing ``ensure_chat_loaded`` to construct supergroup/channel entities and fixing CLI option flag parsing across message subcommands.
* Resolved ``free(): double free detected in tcache 2`` runtime crash on exit by thread-guarding json-c reference counts and ordering thread cleanup before client teardown.
* Fixed missing ``deleteMessages`` API request dispatch in ``App::cmd_msg_delete``, ensuring message deletions are sent to Telegram servers before logging success.
* Added inline flag parsing support for ``--folder=<id>`` and ``-F=<id>`` in ``cmd_chat_ls``.
* Fixed RFC 8259 compliance in ``escape_json_string`` by formatting ASCII control characters (``< 0x20``) as ``\u00XX`` hex unicode escape sequences.

[0.6.0] — 2026-08-06
====================

.. rubric:: Added

* Chat Folder Management CLI suite (``grm folder ls``, ``create``, ``edit``, ``delete``) supporting category filter flags (``--include-groups``, ``--include-channels``, ``--include-bots``, ``--include-contacts``, ``--include-non-contacts``, ``--exclude-muted``, ``--exclude-read``, ``--exclude-archived``) and explicit chat ID inclusions/exclusions/pins.
* Chat List Folder Filtering via ``grm chat ls --folder <id>`` / ``-F <id>`` flag to inspect chats scoped to a specific ``chatListFolder``.
* Automated ``updateChatFolders`` event caching in ``TdClient`` with fallback probing for instant folder discovery.
* Human TTY color badges, AI Markdown, and AI JSON envelope formatting for chat folder summaries.
* Development Workflow & Engineering Procedures specification in ``docs/project/development-workflow.rst`` defining step-by-step Feature Addition (9-step protocol) and Bug Repair (7-step protocol) procedures.
* Unit test suite ``tests/test_folder.cpp`` verifying chat folder spec registration and multi-format rendering.

.. rubric:: Changed

* Registered ``get_folder_spec()`` in ``CommandRegistry`` to automatically power ``grm folder --help``, ``grm -F json --help``, and shell completion generators.

.. rubric:: Fixed

* Resolved TDLib asynchronous network queue flush bug on CLI termination. Added network flush grace window in ``TdClient::stop()`` to guarantee pending outgoing MTProto messages and requests are delivered before TDLib client session shutdown.
* Added ``tests/test_td_client_flush.cpp`` unit test suite to validate ``TdClient`` shutdown timing (>= 1000ms) and stop idempotency.

[0.5.1] — 2026-08-06
====================

.. rubric:: Added

* Custom Fedora OCI container image (``Containerfile.ci``) pre-baking C++23 build dependencies, documentation validators (``rstcheck``, ``python3-docutils``), and TDLib 1.8.66 libraries/headers for zero-delay CI execution.
* Server-side search integration across all listing commands: ``grm chat ls --filter`` (via ``searchChats``), ``grm topic ls --filter`` (via ``getForumTopics(query)``), and ``grm msg ls --filter`` / ``grm msg search`` (via ``searchChatMessages``).
* Universal ``ListOptions`` parser with default limit 20, relative/ISO duration cutoff (``-S | --since``), multi-field regex filter (``-f | --filter``), and reverse ordering (``-r | --reverse``).
* Relative and ISO timestamps in verbose (``-v`` / ``--verbose``) human mode, AI Markdown tables, and AI JSON envelope formats for chat and topic listings.
* Comprehensive Planned Features & Architectural Roadmap specifications in ``docs/user/grm.rst`` and ``docs/man/grm.1.rst`` covering Chat Folder CRUD (``grm folder``), Universal Cross-Domain Search (``grm search``), Media/File Filtering by Type (``grm file``), Contact CRUD (``grm contact``), Silent & Scheduled Delivery (``grm msg send``), Audio Processing Engine with **libsox** (``grm audio``), and Inline Media vs. Raw Document Attachment modes.
* Sender name format configuration (``sender_name_format``: ``username`` | ``fullname``) defaulting to ``username`` (with fallback to display name).
* CLI flags ``-N | --name-format <username|fullname>``, ``-u | --username``, and ``--full-name`` for name format selection.
* Message date range filtering via ``-S | --since <duration|date>`` flag in ``grm msg ls`` and ``grm msg search`` (supporting relative durations like ``1d``, ``2h``, ISO dates, and Unix timestamps).
* Automatic sender name resolution for message list and search commands across all output formats.
* Dynamic RPATH configuration linking against local ``libtdjson.so.1.8.66`` in ``~/.local/lib``.

.. rubric:: Changed

* Updated ``.gitlab-ci.yml`` pipeline architecture to execute inside the pre-built custom Fedora OCI image (``registry.gitlab.com/renich/grm/ci-fedora:latest``) with global ``PKG_CONFIG_PATH`` and ``LD_LIBRARY_PATH`` environment variables.
* Upgraded TDLib compatibility layer to support TDLib 1.8.66 / MTProto Layer 228 API specifications.
* Gated ``[MsgID <id>]`` display prefix in message list output to verbose (``-v`` / ``--verbose``) mode.
* Flattened ``setTdlibParameters`` request payload structure for modern TDLib release standards.

.. rubric:: Fixed

* Fixed CMake library search paths in ``CMakeLists.txt`` by adding global ``link_directories`` for ``TDJSON_LIBRARY_DIRS`` and ``JSONC_LIBRARY_DIRS``, resolving linker errors on CTest test targets.
* Fixed CMake version compatibility policy error for TDLib by explicitly setting ``-DCMAKE_POLICY_VERSION_MINIMUM=3.5`` for modern CMake toolchains (3.31+).
* Fixed GitLab CI YAML script syntax parsing in release stages by formatting script steps with block scalar multiline syntax.
* Fixed deep history pagination timeouts in large channels by leveraging TDLib's server-side search index (``searchChatMessages``) with fallback to ``getChatHistory`` capped at 15 batch iterations.
* Fixed loop termination in message listing pagination to halt immediately when target limit or since cutoff date is reached.
* Fixed instant MTProto stream synchronization by issuing non-blocking ``openChat`` TDLib requests inside ``ensure_chat_loaded``.

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
