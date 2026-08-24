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

-H, --help=all
   Show exhaustive help screen for all commands and subcommands.

-V, --version
   Display version, git commit, and build environment information.

-v, --verbose
   Enable verbose TDLib log output to stdout.

-d, --debug
   Enable low-level debug tracing and JSON payload logging.

-q, --quiet
   Suppress informational messages and output only errors or data.

-c, --config <file>
   Specify alternative path to configuration file (default: ``~/.config/grm/config.json``).

-T, --test-dc
   Connect to Telegram Test Data Center (DC) environment.

-F, --format <fmt>
   Set output format: ``human``, ``markdown``, ``json``, or ``plain`` (default: ``auto``). Note: JSON format outputs compact single-line JSON by default.

-p, -P, --pretty
   Pretty-print JSON output with multi-line indentation and spacing (when used with ``-F json``).

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

grm login [-p|--phone <num>] [-k|--code <code>] [-q|--qr]
   Authenticate against Telegram servers using terminal phone code flow or browser QR code.

   -p, --phone <num>
      International phone number (e.g. ``+523330000000``).

   -k, --code <code>
      SMS or Telegram login code.

   -q, --qr
      Authenticate via QR code by generating an interactive HTML QR code page and opening it via ``xdg-open``.

grm logout
   Log out from Telegram, terminate the active TDLib session, and clear local session state.

Chat & Group Management
-----------------------

grm chat ls [-n|--limit <N>] [-S|--since <duration|date>] [-f|--filter <pattern>]
   List active chats, supergroups, channels, and private conversations.

   -n, --limit <N>
      Maximum number of chats to list (default: 100).

   -S, --since <duration|date>
      Filter chats active since natural duration (e.g. ``"1 day ago"``, ``"3 days ago"``, ``"2h"``), ISO date, or timestamp.

   -f, --filter <pattern>
      Filter chats by title, type, or ID (case-insensitive substring or regex). Can be passed multiple times.

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

grm chat pin <chat_id>
   Pin a chat or group at the top of the chat list.

grm chat unpin <chat_id>
   Unpin a chat or group from the top of the chat list.

grm chat delete <chat_id>
   Delete chat history or leave group.

Message Lifecycle
-----------------

grm msg ls [-n|--limit <N>] [-t|--topic <id>] [-S|--since <duration|date>] [-f|--filter <pattern>] [-r|--reverse] <chat_id>
   List messages from a chat or forum topic in chronological order (oldest first at top, newest at bottom).

   -n, --limit <N>
      Maximum number of messages to display (default: 20).

   -t, --topic <id>
      Filter messages by forum topic ID.

   -S, --since <duration|date>
      Filter messages since natural human/systemd duration (e.g. ``"1 day ago"``, ``"3 days ago"``, ``"1 month ago"``, ``"2h"``, ``"30m"``, ``"yesterday"``, ``"today"``), ISO date (``YYYY-MM-DD``), or Unix timestamp.

   -f, --filter, --sender <pattern>
      Filter messages by sender username, display name, or handle (case-insensitive substring or regex). Can be passed multiple times (e.g. ``--filter="renichbon" --filter="k_brown"``) to match any specified sender.

   -r, --reverse
      Display messages in reverse-chronological order (newest first at top).

grm msg pin <chat_id> <message_id>
   Pin a specific message in a chat or supergroup.

grm msg unpin <chat_id> <message_id>
   Unpin a specific message in a chat or supergroup.

grm msg unpin-all <chat_id>
   Unpin all pinned messages in a chat or supergroup.

grm msg export [-f|--format csv|json] [-o|--output <file>] [-t|--topic <id>] <chat_id>
   Export chat or topic history to a CSV or JSON file.

   -f, --format <fmt>
      Export file format: ``csv`` or ``json``.

   -o, --output <file>
      Output target file path.

   -t, --topic <id>
      Target specific forum topic ID.

grm msg search [-q|--query "<query>"] [-n|--limit <N>] [-t|--topic <id>] [-S|--since <duration|date>] [-f|--filter <pattern>] <chat_id>
   Search message history using query pattern or regex.

   -q, --query "<query>"
      Regex search pattern.

   -n, --limit <N>
      Maximum matching messages to return.

   -t, --topic <id>
      Target specific forum topic ID.

   -S, --since <duration|date>
      Filter messages since natural human/systemd duration (e.g. ``"1 day ago"``, ``"3 days ago"``, ``"1 month ago"``, ``"2h"``, ``"30m"``, ``"yesterday"``, ``"today"``), ISO date (``YYYY-MM-DD``), or Unix timestamp.

   -f, --filter, --sender <pattern>
      Filter messages by sender username, display name, or handle (case-insensitive substring or regex).

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

grm topic ls [-n|--limit <N>] [-S|--since <duration|date>] [-f|--filter <pattern>] <supergroup_id>
   List active forum topics in a Telegram supergroup.

   -n, --limit <N>
      Maximum topics to list (default: 100).

   -S, --since <duration|date>
      Filter topics active since natural duration (e.g. ``"1 day ago"``, ``"3 days ago"``, ``"2h"``), ISO date, or timestamp.

   -f, --filter <pattern>
      Filter topics by name or thread ID (case-insensitive substring or regex). Can be passed multiple times.

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

Story Management (CRUD)
------------------------

grm story post [--photo <path>|--video <path>] [--caption <caption>] [--privacy everyone|contacts|close_friends] [--period <time>] [--pinned] [--protect] [-C|--chat <id>]
   Publish a new photo or video story. Supports Markdown hyperlink entity formatting in captions.

   -p, --photo <path>
      Path to local photo image file to publish as story.

   -v, --video <path>
      Path to local video file to publish as story.

   -c, --caption <caption>
      Story caption (supports Markdown hyperlinks and formatting).

   --privacy <setting>
      Privacy setting: ``everyone`` (default), ``contacts``, or ``close_friends``.

   --period <time>
      Active story duration: ``6h``, ``12h``, ``24h`` (default), or ``48h``.

   --pinned
      Pin story to chat page / profile under the permanent "Posts" tab (sets ``is_posted_to_chat_page: true``).

   --protect
      Protect story content from saving and forwarding.

   -C, --chat <id>
      Target chat or channel ID (default: personal account).

grm story edit -s|--story-id <id> [--photo <path>|--video <path>] [--caption <caption>] [-C|--chat <id>]
   Edit media content or caption of an existing story.

   -s, --story-id <id>
      Story identifier to edit.

   -p, --photo <path>
      New photo image file for story.

   -v, --video <path>
      New video file for story.

   -c, --caption <caption>
      New story caption (supports Markdown hyperlinks and formatting).

   -C, --chat <id>
      Target chat or channel ID (default: personal account).

grm story ls [-C|--chat <id>] [-n|--limit <N>] [-p|--pinned] [-a|--archived] [-A|--all]
   List active stories, pinned profile posts, and archived stories.

   -C, --chat <id>
      Target chat or channel ID (default: personal account).

   -n, --limit <N>
      Maximum number of stories to display (default: 20).

   -p, --pinned, --posts
      List stories posted to profile / chat page.

   -a, --archived
      List archived stories.

   -A, --all
      List active, pinned profile, and archived stories combined.

grm story delete -s|--story-id <id> [-C|--chat <id>]
   Delete a published story by identifier.

   -s, --story-id <id>
      Story identifier to delete.

   -C, --chat <id>
      Target chat or channel ID (default: personal account).

Emoji Status Management
-----------------------

grm status ls [-f|--filter <query>] [-r|--recent] [-p|--packs] [<query>]
   List and search available default, recent, and custom status emojis across installed packs (including official ``Animated Emoji``).

   -f, --filter <query>
      Filter emojis by unicode character, description, pack name, or ID.

   -r, --recent
      Show recent custom emoji statuses.

   -p, --packs
      Show installed custom emoji packs.

grm status set -e|--emoji <id> [-d|--duration <time>] [-C|--chat <id>]
   Set a custom Telegram emoji status badge.

   -e, --emoji <id>
      Custom Telegram emoji identifier (int64 string).

   -d, --duration <time>
      Status badge duration (e.g. ``30m``, ``1h``, ``8h``, ``2d``, ``1w``).

   -C, --chat <id>
      Target boosted channel or supergroup chat ID.

grm status clear [-C|--chat <id>]
   Clear active custom emoji status badge.

   -C, --chat <id>
      Target boosted channel or supergroup chat ID.

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

5. Publish a Telegram Story with clickable URL and pin to profile page:

   .. code-block:: bash

      grm story post --photo assets/logo.png --caption "grm ya soporta historias 🚀 https://gitlab.com/renich/grm" --pinned --period 24h

6. Edit an existing story's caption and media:

   .. code-block:: bash

      grm story edit --story-id 32 --caption "Updated caption with link: https://gitlab.com/renich/grm"

7. List active, profile, and archived stories:

   .. code-block:: bash

      grm story ls --all

8. Search and set a 3D animated emoji status badge for 8 hours:

   .. code-block:: bash

      # Search for laptop emojis across installed packs
      grm status ls 💻

      # Set 3D Animated Laptop badge for 8 hours
      grm status set --emoji 5431376038628171216 --duration 8h

Planned Features & Architectural Roadmap
========================================

The following feature specifications define the ergonomic CLI design, subcommand hierarchy, and TDLib integration architecture for **grm**:

1. Chat Folder Management (Chat Filters & Folders CRUD)
-------------------------------------------------------

Provides full lifecycle management for Telegram chat folders and custom organization filters via TDLib's ``createChatFolder``, ``editChatFolder``, and ``deleteChatFolder`` APIs:

* ``grm folder ls``: List all defined chat folders, included chat categories (groups, channels, bots), excluded statuses (muted/read/archived), and pinned chat IDs. Rendered with color-coded folder badges in human TTY view.
* ``grm folder create <title> [--include-groups] [--include-channels] [--include-chats <ids...>]``: Create a custom folder filter.
* ``grm folder edit <folder_id> [--title <title>] [--add-chat <id>] [--remove-chat <id>]``: Modify folder title or chat memberships.
* ``grm folder delete <folder_id>``: Remove a chat folder.
* **Chat Listing Ergonomics**: Adds the ``--folder <id>`` option to ``grm chat ls`` (e.g. ``grm chat ls --folder 2``) to restrict chat listings to specific folder contents.

2. Universal Cross-Domain Search
--------------------------------

Unified server-side and local search across all Telegram entity domains with ANSI term highlighting in human TTY mode and structured tabular output:

* ``grm search chats <query>``: Global search across public and private chats, supergroups, and broadcast channels via ``searchChats`` and ``searchPublicChats``.
* ``grm search msgs <query> [-c <chat_id>] [-t <type>]``: Global or per-chat message search across text content, sender handles, and media captions via ``searchMessages`` and ``searchChatMessages``.
* ``grm search users <query|phone|handle>``: Search users and public profiles by handle, phone number, or display name via ``searchContacts`` and ``searchUserByUsername``.
* ``grm search "<query>"``: Multi-domain universal search aggregating matching chats, contacts, and recent messages in a single grouped view.

3. Media & File Filtering/CRUD by Type
--------------------------------------

Granular media listing and extraction categorized by specific Telegram media filters (``SearchMessagesFilter``):

* ``grm file ls <chat_id> [--type photo|video|doc|audio|voice|url|all] [-n limit]``: List media attachments in a chat or topic thread filtered by type (photos, videos, documents, music audio, voice notes, video notes, URLs).
* ``grm file get <chat_id> <file_id|msg_id> [-o <path>]``: Download a specific document or media attachment by file ID or message ID with live progress rendering.
* ``grm file download-all <chat_id> [--type photo|video|doc|audio|all] [-o <dir>]``: Bulk download all media matching specified media types with non-blocking thread pool execution.

4. Contact Management (Contacts CRUD)
-------------------------------------

Complete management for Telegram contact lists and user profiles:

* ``grm contact ls``: List all saved Telegram contacts and their online availability status (e.g. ``Online``, ``2m ago``) via ``getContacts``.
* ``grm contact info <user_id|phone|handle>``: Inspect user profile details, bio, phone number, and restrictions via ``getUser`` and ``getUserFullInfo``.
* ``grm contact add <phone> <first_name> [last_name] [--share-phone]``: Import or add a new user to contacts via ``addContact`` and ``importContacts``.
* ``grm contact delete <user_id>``: Remove user from contacts list via ``removeContacts``.

5. Silent & Scheduled Messages
------------------------------

Advanced message delivery controls utilizing TDLib's ``messageSendOptions``:

* ``grm msg send <chat_id> "<message>" [--silent] [--schedule-at "<datetime|duration>"] [--send-when-online]``:

  * ``--silent`` / ``-s``: Delivers the message silently without triggering sound or vibration notifications (``disable_notification: true``).
  * ``--schedule-at "<datetime|duration>"``: Schedules delivery at a specific date/time (e.g. ``"2026-08-07T09:00:00Z"``) or natural language relative duration (e.g. ``"in 2 hours"``, ``"tomorrow 9am"``) via ``messageSchedulingStateSendAtDate``.
  * ``--send-when-online``: Delivers private chat messages automatically when the recipient comes online via ``messageSchedulingStateSendWhenOnline``.

* ``grm msg scheduled ls <chat_id>``: View pending scheduled messages in a chat or topic thread.
* ``grm msg scheduled delete <chat_id> <message_id>``: Cancel and delete a pending scheduled message.

6. Audio CRUD & Media Processing Engine
---------------------------------------

First-class audio transmission and voice note processing:

* ``grm audio voice <chat_id> <voice_ogg_path> [--convert] [--silent]``: Send voice notes (OGG/Opus format) via ``inputMessageVoiceNote``.
* **SoX / libsox Engine Integration**: Native integration with **libsox** (or ``sox`` CLI engine) to automatically probe audio metadata, convert arbitrary audio formats to OGG/Opus voice notes, and extract exact 100-bar amplitude waveform arrays for TDLib voice note rendering.
* ``grm audio info <file_path>``: Inspect audio duration, sample rate, channels, and codec details via libsox.

7. Introspective Shell Completion Engine (``grm completion``)
--------------------------------------------------------------

Self-generating, zero-maintenance shell completion generator powered by native C++ command tree introspection:

* ``grm completion bash``: Output context-aware Bash completion script to stdout (supports ``source <(grm completion bash)`` and automatic installation via ``make install-user``).
* ``grm completion zsh``: Output native Zsh completion function script with rich subcommand descriptions.
* ``grm completion fish``: Output Fish shell completion definitions to stdout.
* **Dynamic Completion Wrapper**: Per FreeDesktop/FHS standards, ``~/.local/share/bash-completion/completions/grm`` is installed as a lightweight 10-line wrapper script (``_grm_loader``) that dynamically executes ``grm completion bash`` on demand. You **never** need to re-install or update the completion file when subcommands or flags change in new builds of **grm**.

8. Modular Single-Source Command Registry (``CommandRegistry``)
-------------------------------------------------------------------------

Domain-driven, modular command specification architecture (``include/grm/command_registry.hpp``):

* **Modular Feature Self-Registration**: Every feature module (``src/cmd_auth.cpp``, ``src/cmd_chat.cpp``, ``src/cmd_msg.cpp``, ``src/cmd_topic.cpp``, ``src/cmd_file.cpp``, ``src/cmd_completion.cpp``) owns and exports its own ``CommandSpec`` data structure containing its subcommands, synopsis, option flags, descriptions, and allowed parameter choices (e.g. ``photo|video|doc|audio|all`` for ``--type``).
* **Automatic Discovery & Aggregation**: ``CommandRegistry`` aggregates feature specs directly from module accessors without maintaining separate text files or duplicate registry files.
* **Dual Automatic Rendering**:

  * **Help Engine**: ``CommandRegistry::render_help()`` and ``render_all_help()`` dynamically format all CLI ``--help`` screens and exhaustive master reference manuals with consistent padding and option alignment.
  * **Completion Engine**: ``CommandRegistry::render_completion()`` automatically generates context-aware shell autocompletions for Bash, Zsh, and Fish directly from the feature definitions.

9. Attachment Types: Inline Media vs. Raw Document
--------------------------------------------------

Explicit distinction between visual inline media transmission and uncompressed binary document transfers:

* **Inline Media (``--photo``, ``--video``, ``--animation``)**: Transmitted as compressed visual media (via ``inputMessagePhoto`` or ``inputMessageVideo``) featuring automatic inline thumbnail previews and streaming optimization.
* **Uncompressed Document (``--file`` / ``--doc``)**: Transmitted as raw binary document attachments (via ``inputMessageDocument``) preserving byte-for-byte fidelity without compression or metadata strip.

Project Roadmap & Status
========================

The following features and specifications have been implemented and verified:

- [x] **Single-Source Command Registry**: Unified ``CommandRegistry`` powering ``--help`` and ``grm completion``.
- [x] **Introspective Shell Completion**: Native ``grm completion <bash|zsh|fish>`` engine with dynamic loader wrapper script.
- [x] **GNU Standard Makefile**: Default ``PREFIX ?= /usr/local`` (per GNU Coding Standards Section 7.2.5) with ``PREFIX_USER ?= $(HOME)/.local`` for non-root ``make install-user``.
- [x] **File Engine Consolidation**: Unified ``grm file get [-a|-A|--all] <chat_id> [<message_ids...>]`` syntax.
- [x] **Message Pin & Unpin**: Consolidated ``grm msg unpin [-a|--all] <chat_id> [<message_ids...>]``.

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

Per **FHS 3.0** and the **XDG Base Directory Specification**, install **grm** into your user profile without root privileges (defaults to ``PREFIX_USER ?= ~/.local``):

.. code-block:: bash

   # Installs binary to ~/.local/bin/grm, man page to ~/.local/share/man/man1/grm.1, and completion wrapper
   make install-user

   # Ensure ~/.local/bin is present in your PATH:
   export PATH="$HOME/.local/bin:$PATH"

System Installation (Requires Sudo)
-----------------------------------

Per **GNU Coding Standards Section 7.2.5**, system installation defaults to ``PREFIX ?= /usr/local``:

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
   User Bash tab autocompletion dynamic wrapper script.

See Also
========

**rst2man**\(1), **sphinx-build**\(1), **tdlib**\(7)
