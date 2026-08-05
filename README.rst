==================================
grm — Group & Telegram Manager CLI
==================================

**grm** is a high-performance, native **C++23** command-line tool for managing Telegram accounts, groups, message history, forum topics, file uploads, media streaming, and data extractions. Powered directly by Telegram's official C++ engine **TDLib** (``libtdjson``).

Dual-Optimized Interface Architecture
-------------------------------------

``grm`` is specifically architected to serve as a seamless bridge for both **human developers/sysadmins** and **autonomous AI agents / LLM workflows**:

- **For Human Users**:
  - Intuitive, POSIX-compliant CLI command structure (``grm chat``, ``grm topic``, ``grm msg``, ``grm file``).
  - Humanized output formatting with structured log levels (``[INFO]``, ``[AUTH]``, ``[DEBUG]``).
  - Interactive phone number and 2FA passcode login flow.
  - Native Telegram Rich Text Markdown (V2) support for sending formatted text (bold, italic, code blocks, links).
  - Forum Topic custom emoji icon customization (``-e | --emoji``).

- **For AI Agents & Automation**:
  - Machine-readable JSON export formats (``grm msg export -f json``).
  - Deterministic POSIX exit codes and zero-fluff error reporting.
  - Full supergroup ID resolution (``-100...``) avoiding MTProto ``AccessHash`` invalid peer errors.
  - Asynchronous non-blocking TDLib event loop suitable for headless execution and subagent integration.

Key Features
------------

- **Official TDLib Engine**: Direct C++ integration with ``libtdjson`` for complete, zero-hallucination MTProto coverage.
- **Full CRUD Operations**: Complete lifecycle management for Chats, Groups, Channels, Supergroup Forum Topics, Messages, and Files.
- **Telegram Rich Formatting**: Native entity parsing (Markdown V2 / HTML) via TDLib's ``parseTextEntities``.
- **File Upload & Media Engine**: Bulk document and media attachment upload with automatic MIME detection and progress tracking.
- **File Download Engine**: Download specific message attachments or bulk download all media from chats and forum topics.
- **Strict Quality Suite**: 100% C++23 standards compliance audited with Clang-Tidy, Ameba/Flaw conventions, and CTest integration.

Directory Layout
----------------

.. code-block:: text

   grm/
   ├── CMakeLists.txt              # C++23 CMake build configuration
   ├── GNUmakefile                 # Build, test, lint, and install targets
   ├── README.rst                  # Project documentation
   ├── docs/                       # Functional, technical specs, and ADRs
   ├── include/
   │   └── grm/
   │       ├── app.hpp             # Main CLI application controller
   │       ├── config.hpp          # Configuration & CLI option loader
   │       ├── exporter.hpp        # CSV and JSON chat history exporter
   │       ├── json_utils.hpp      # C++23 RAII JsonValue wrapper for json-c
   │       ├── logger.hpp          # Structured level logger ([INFO], [AUTH], etc.)
   │       ├── td_client.hpp       # Asynchronous TDLib client wrapper
   │       └── uploader.hpp        # Document and file payload builder
   ├── src/                        # Implementation sources
   └── tests/                      # Unit and integration tests

Prerequisites (Fedora Linux)
----------------------------

Install required build dependencies:

.. code-block:: bash

   sudo dnf install tdlib-devel json-c-devel cmake ninja-build gcc-c++ clang-tools-extra

Building & Testing
------------------

Using the ``GNUmakefile``:

.. code-block:: bash

   # Development build (unoptimized, -O0 -g3 debug symbols)
   make

   # Optimized production build (-O3 -DNDEBUG, stripped)
   make release

   # Run test suite
   make check

   # Run static analyzer and linters
   make lint
   make analyze

   # Build man page and check RST documentation syntax
   make man
   make doc-check

   # Install binary and man page for current user (~/bin/grm and ~/.local/share/man/man1/grm.1)
   make install-user
   make install-user-man

Documentation & Man Page
------------------------

- **User Guide**: Comprehensive reStructuredText documentation in ``docs/user/grm.rst``.
- **Man Page**: Source maintained in ``docs/man/grm.1.rst``, compiled with ``rst2man`` to ``build/grm.1``.
- **System Installation**: Run ``sudo make install`` to install ``grm`` to ``/usr/local/bin/grm`` and man page to ``/usr/local/share/man/man1/grm.1``.
- **User Installation**: Run ``make install-user`` and ``make install-user-man`` to install binary to ``~/bin/grm`` and man page to ``~/.local/share/man/man1/grm.1``. View with ``man grm``.

Command Reference
-----------------

Global Options
~~~~~~~~~~~~~~

- ``-h, --help``: Display help screen and exit.
- ``-V, --version``: Display version information and exit.
- ``-v, --verbose``: Enable verbose TDLib state output.
- ``-d, --debug``: Enable raw JSON payload tracing.
- ``-q, --quiet``: Suppress non-essential informational output.
- ``-c, --config <path>``: Custom configuration file path.
- ``-T, --test-dc``: Connect to Telegram Test Data Center environment.

CRUD Command Structure
~~~~~~~~~~~~~~~~~~~~~~

1. **Chats & Groups (`grm chat`)**:
   - ``grm chat create group [--private|--public] "<title>"``
   - ``grm chat create channel [--private|--public] "<title>" ["<description>"]``
   - ``grm chat ls [-n|--limit <N>]``
   - ``grm chat info <chat_id>``
   - ``grm chat set-title <chat_id> "<new_title>"``
   - ``grm chat set-desc <chat_id> "<description>"``
   - ``grm chat pin <chat_id> <message_id>``
   - ``grm chat unpin <chat_id> [<message_id>]``
   - ``grm chat delete <chat_id>``

2. **Supergroup Forum Topics (`grm topic`)**:
   - ``grm topic create <supergroup_id> "<topic_name>" [--icon-color <color>] [-e|--emoji <id>]``
   - ``grm topic ls [-n|--limit <N>] <supergroup_id>``
   - ``grm topic info <supergroup_id> <topic_id>``
   - ``grm topic edit <supergroup_id> <topic_id> [--name "<new_name>"] [-e|--emoji <id>]``
   - ``grm topic close <supergroup_id> <topic_id>``
   - ``grm topic reopen <supergroup_id> <topic_id>``
   - ``grm topic pin <supergroup_id> <topic_id>``
   - ``grm topic unpin <supergroup_id> <topic_id>``
   - ``grm topic delete <supergroup_id> <topic_id>``

3. **Messages & Attachments (`grm msg`)**:
   - ``grm msg send [-a|--attach <file>] [-m|--media] [-C|--caption "<text>"] [-t|--topic <id>] <chat_id> ["<message>"]``
   - ``grm msg ls [-t|--topic <id>] [-n|--limit <N>] <chat_id>``
   - ``grm msg info <chat_id> <message_id>``
   - ``grm msg search [-t|--topic <id>] [-q|--query "<pattern>"] [-n|--limit <N>] <chat_id>``
   - ``grm msg export [-f|--format csv|json] [-o|--output <file>] [-t|--topic <id>] [-n|--limit <N>] <chat_id>``
   - ``grm msg edit [-t|--topic <id>] <chat_id> <message_id> "<new_text>"``
   - ``grm msg delete [--for-everyone] <chat_id> <message_ids...>``

4. **File Downloads (`grm file`)**:
   - ``grm file get [-o|--output <dir|file>] [-t|--topic <id>] <chat_id> <message_ids...>``
   - ``grm file download-all [-o|--output <dir>] [-t|--topic <id>] [-n|--limit <N>] [--type photo|video|doc|audio|all] <chat_id>``

Usage Examples
~~~~~~~~~~~~~~

1. **Send Message with Rich Formatting**:
   .. code-block:: bash

      grm msg send -t 2 -1003750297693 "Hello **World**! Check https://evalinux.com."

2. **Send Document or Photo Media Attachment**:
   .. code-block:: bash

      grm msg send -a /tmp/report.pdf -C "Monthly Financial Report" -t 2 -1003750297693
      grm msg send -m -a /tmp/chart.png -C "Growth Chart" -t 2 -1003750297693

3. **Create Forum Topic with Custom Emoji Icon**:
   .. code-block:: bash

      grm topic create -e 5368560552786271734 -1003750297693 "AI Agent Logs"

4. **Export Messages as JSON for AI Processing**:
   .. code-block:: bash

      grm msg export -f json -o history.json -t 2 -1003750297693

Authors & Credits
-----------------

- **Principal Architect & Maintainer**: Rénich Bon Ćirić (<renich@evalinux.com>)
- **AI Systems & Pair Programming Partner**: Antigravity AI (Google DeepMind)
