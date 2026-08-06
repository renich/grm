===============================
GRM - Group & Telegram Manager
===============================

----------------------------------------------------
C++23 / TDLib Telegram CLI User & Operator Guide
----------------------------------------------------

:Authors: Rénich Bon Ćirić & Antigravity AI (Google DeepMind)
:Contact: renich@evalinux.com

:Manual section: 1
:Manual group: User Commands


Description
===========

**grm** is a high-performance, asynchronous CLI manager for Telegram built with modern C++23 and TDLib. It provides comprehensive CRUD (Create, Read, Update, Delete) management for private chats, basic groups, supergroups, channels, and forum topics, alongside a robust file and media download engine.

Features
--------

* **Full Authentication Flow**: Interactive or non-interactive phone code and 2FA password verification.
* **Complete CRUD Lifecycle**: Full lifecycle support for chats, groups, forum topics, and text/media messages.
* **File Download Engine**: Batch download media attachments from chat history or specific forum topics with type filtering.
* **Context-Aware Shell Autocompletion**: Full Bash tab completion covering commands, subcommands, options, and parameters.
* **Telegram Rich Text & Emoji Icons**: Native Telegram Markdown V2 entity parsing and custom Supergroup topic emoji icons.
* **Multiple Output Formats**: Supports human-readable tables, ANSI color, Markdown, JSON, and CSV export.
* **POSIX & GNU Standard Compliance**: Supports GNU short/long option flags and positional parameters.

Synopsis
========

.. code-block:: text

   grm [global-options] <command> [<subcommand>] [options] [args]

Global Options
==============

-h, --help
   Show command or subcommand help message and exit.

-V, --version
   Display version, git commit, and build environment information.

-v, --verbose
   Enable verbose TDLib log output to stdout.

-d, --debug
   Enable low-level debug tracing and JSON payload logging.

-q, --quiet
   Suppress informational messages and output only errors or data.

-c, --config <file>
   Specify alternative path to configuration file (default: ``~/.config/grm/grm.conf``).

-T, --test-dc
   Connect to Telegram Test Data Center (DC) environment.

-F, --format <fmt>
   Set output format: ``human``, ``markdown``, ``json``, or ``plain`` (default: ``auto``).

-N, --name-format <fmt>
   Set sender name display format: ``username`` (default, e.g. ``@username``) or ``fullname`` (e.g. ``First Last``).

-u, --username
   Shortcut to set sender name format to ``username``.

--full-name
   Shortcut to set sender name format to ``fullname``.

--color <mode>
   Control ANSI color output: ``auto``, ``always``, or ``never`` (or ``--no-color``).

Commands & Subcommands
======================

Authentication
--------------

grm login [-p|--phone <num>] [-k|--code <code>]
   Authenticate against Telegram servers using phone number, login code, and optional 2FA password.

   -p, --phone <num>
      International phone number (e.g. ``+523330000000``).

   -k, --code <code>
      SMS or Telegram login code.

Chat & Group Management
-----------------------

grm chat ls [-n|--limit <N>]
   List active chats, supergroups, channels, and private conversations.

   -n, --limit <N>
      Maximum number of chats to list (default: 100).

grm chat create group [--private|--public] "<title>"
   Create a new basic group or supergroup.

grm chat create channel [--private|--public] "<title>" ["<description>"]
   Create a new broadcast channel.

grm chat info <chat_id>
   Display detailed JSON metadata for a specific chat or group.

grm chat set-title <chat_id> "<new_title>"
   Update the title of a group or channel.

grm chat set-desc <chat_id> "<description>"
   Update the description of a supergroup or channel.

grm chat pin <chat_id> <message_id>
   Pin a message in a chat or supergroup.

grm chat unpin <chat_id> [<message_id>]
   Unpin a message in a chat or supergroup.

grm chat delete <chat_id>
   Delete chat history or leave group.

Message Lifecycle
-----------------

grm msg ls [-n|--limit <N>] [-t|--topic <id>] [-S|--since <duration|date>] <chat_id>
   List recent messages from a chat or forum topic.

   -n, --limit <N>
      Maximum number of messages to display (default: 20).

   -t, --topic <id>
      Filter messages by forum topic ID.

   -S, --since <duration|date>
      Filter messages since relative duration (e.g. ``1d``, ``2h``, ``30m``), ISO date (``YYYY-MM-DD``), or Unix timestamp.

grm msg export [-f|--format csv|json] [-o|--output <file>] [-t|--topic <id>] <chat_id>
   Export chat or topic history to a CSV or JSON file.

   -f, --format <fmt>
      Export file format: ``csv`` or ``json``.

   -o, --output <file>
      Output target file path.

   -t, --topic <id>
      Target specific forum topic ID.

grm msg search [-q|--query "<query>"] [-n|--limit <N>] [-t|--topic <id>] [-S|--since <duration|date>] <chat_id>
   Search message history using query pattern or regex.

   -q, --query "<query>"
      Regex search pattern.

   -n, --limit <N>
      Maximum matching messages to return.

   -t, --topic <id>
      Target specific forum topic ID.

   -S, --since <duration|date>
      Filter messages since relative duration (e.g. ``1d``, ``2h``, ``30m``), ISO date (``YYYY-MM-DD``), or Unix timestamp.

grm msg send [-a|--attach <file>] [-m|--media] [-C|--caption "<text>"] [-t|--topic <id>] <chat_id> ["<message>"]
   Send a text message, document, or media file attachment. Supports Telegram Rich Text Markdown V2 formatting.

   -a, --attach <file>
      Path to local file attachment (repeatable).

   -m, --media
      Send attachment as compressed media (photo/video).

   -C, --caption "<text>"
      Caption text for file attachment.

   -t, --topic <id>
      Target specific forum topic ID.

grm msg info <chat_id> <message_id>
   View detailed message JSON metadata.

grm msg edit [-t|--topic <id>] <chat_id> <message_id> "<new_text>"
   Edit sent text message content.

grm msg delete [--for-everyone] <chat_id> <message_ids...>
   Delete one or more messages.

   -e, --for-everyone
      Delete message for all participants in group.

Forum Topic Management
----------------------

grm topic ls [-n|--limit <N>] <supergroup_id>
   List active forum topics in a Telegram supergroup.

   -n, --limit <N>
      Maximum topics to list (default: 100).

grm topic create [-e|--emoji <id>] [--icon-color <color>] <supergroup_id> "<topic_name>"
   Create a new forum topic in a supergroup with optional custom emoji icon.

   -e, --emoji, --icon <id>
      Custom Telegram emoji icon identifier.

   --icon-color <color>
      RGB icon color hex integer (e.g. 0x6FB9F0).

grm topic info <supergroup_id> <topic_id>
   View detailed metadata for a forum topic.

grm topic edit [-e|--emoji <id>] <supergroup_id> <topic_id> ["<new_name>"]
   Rename an existing forum topic or change its custom emoji icon.

   -e, --emoji, --icon <id>
      Custom Telegram emoji icon identifier.

grm topic close <supergroup_id> <topic_id>
   Close a forum topic.

grm topic reopen <supergroup_id> <topic_id>
   Reopen a closed forum topic.

grm topic pin <supergroup_id> <topic_id>
   Pin a forum topic.

grm topic unpin <supergroup_id> <topic_id>
   Unpin a forum topic.

grm topic delete <supergroup_id> <topic_id>
   Delete a forum topic and its history.

File Download Engine
--------------------

grm file get [-A|--all] [-o|--output <dir|file>] [-t|--topic <id>] [-n|--limit <N>] [--type photo|video|doc|audio|all] <chat_id> [<message_ids...>]
   Download specific attachment files by message ID, or bulk download all media files when ``-A`` or ``--all`` is specified.

   -A, --all
      Bulk download all attachment files in chat or topic.

   -o, --output <dir|file>
      Destination directory path or target file path.

   -t, --topic <id>
      Target specific forum topic ID.

   -n, --limit <N>
      Maximum messages to scan during bulk download (default: 100).

   --type <type>
      Filter media type: ``photo``, ``video``, ``doc``, ``audio``, or ``all``.

Examples
========

1. List active chats in table format:

   .. code-block:: bash

      grm chat ls

2. Create a new forum topic with custom emoji icon in supergroup:

   .. code-block:: bash

      grm topic create -e 5368560552786271734 -1003750297693 "DevOps Operations"

3. Send document attachment to topic:

   .. code-block:: bash

      grm msg send -t 2 -a /var/log/syslog -C "System logs" -1003750297693

4. Bulk download all photos from topic:

   .. code-block:: bash

      grm file download-all -o ~/Downloads -t 2 --type photo -1003750297693

Environment Variables
=====================

GRM_FORMAT
   Set default output format: ``human``, ``markdown``, ``json``, ``jsonl`` (or ``ndjson``), or ``plain``.

GRM_COLOR
   Set default ANSI color mode: ``auto``, ``always``, or ``never``.

NO_COLOR
   Disables ANSI color output when present and non-empty.

Git Pre-Push Hook Setup
========================

To register the repository pre-push quality automation hook (runs ``crstlint``, ``make doc-check``, ``make format``, ``make check``, and ``make lint`` prior to pushing to remote repositories):

.. code-block:: bash

   make install-hooks

Installation & Setup
====================

User / Build Dependencies
-------------------------

**grm** requires the following build toolchain and C++ libraries:

* **tdlib-devel** (>= 1.8.0): Telegram C++ JSON interface library (``libtdjson``)
* **json-c-devel** (>= 0.15): JSON parser and serialization engine
* **cmake** (>= 3.25): Build system generator
* **ninja-build** (>= 1.10): Fast build execution engine
* **gcc-c++** / **clang**: C++23 compiler suite
* **python3-docutils**: reStructuredText man page compiler (``rst2man``)

Developer Toolchain Dependencies
--------------------------------

Contributors running static analysis or documentation validation also require:

* **clang-tools-extra** (>= 16.0): ``clang-tidy`` static analyzer and ``clang-format``
* **rstcheck** (>= 6.0): reStructuredText syntax validator

Local User Installation (Non-Root / No Sudo Required)
------------------------------------------------------

Per **FHS 3.0** and the **XDG Base Directory Specification**, install **grm** into your user profile without root privileges:

.. code-block:: bash

   # Installs binary to ~/.local/bin/grm and man page to ~/.local/share/man/man1/grm.1
   make install-user

   # Ensure ~/.local/bin is present in your PATH:
   export PATH="$HOME/.local/bin:$PATH"

System Installation (Requires Sudo)
-----------------------------------

To install **grm** system-wide for all users:

.. code-block:: bash

   make release && sudo make install

Files & Configuration (FreeDesktop XDG Standards)
=================================================

~/.config/grm/config.json
   User configuration file containing API credentials (``api_id``, ``api_hash``) and output format defaults.

~/.local/lib/grm/tdlib_db/
   Persistent TDLib database directory containing encrypted session state, authorization keys, and cache.

~/.local/bin/grm
   User executable installation path per FHS 3.0.

~/.local/share/man/man1/grm.1
   User manual page installation path.

~/.local/share/bash-completion/completions/grm
   User Bash tab autocompletion script.

See Also
========

**rst2man**\(1), **sphinx-build**\(1), **tdlib**\(7)
