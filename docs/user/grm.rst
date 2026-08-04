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

-F, --format <fmt>
   Set output format: ``human``, ``markdown``, ``json``, or ``plain`` (default: ``auto``).

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

grm chat ls
   List active chats, supergroups, channels, and private conversations.

grm chat create <group|channel> [--private|--public] "<title>"
   Create a new group or channel.

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

Message Operations
------------------

grm msg ls [-n|--limit <N>] [-t|--topic <id>] <chat_id>
   List recent messages from a chat or forum topic.

   -n, --limit <N>
      Maximum number of messages to display (default: 50).

   -t, --topic <id>
      Filter messages by forum topic ID.

grm msg export [-f|--format csv|json] [-o|--output <file>] [-t|--topic <id>] <chat_id>
   Export chat or topic history to a CSV or JSON file.

   -f, --format <fmt>
      Export file format: ``csv`` or ``json``.

   -o, --output <file>
      Output target file path.

grm msg search [-q|--query "<query>"] [-n|--limit <N>] [-t|--topic <id>] <chat_id>
   Search message history using query pattern or regex.

grm msg send [-a|--attach <file>] [-m|--media] [-C|--caption "<text>"] [-t|--topic <id>] <chat_id> ["<message>"]
   Send a text message, document, or media file attachment.

   -a, --attach <file>
      Path to local file attachment (repeatable).

   -m, --media
      Send attachment as compressed media (photo/video).

   -C, --caption "<text>"
      Caption text for file attachment.

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

grm topic ls <supergroup_id>
   List active forum topics in a Telegram supergroup.

grm topic create <supergroup_id> "<topic_name>"
   Create a new forum topic in a supergroup.

grm topic info <supergroup_id> <topic_id>
   View detailed metadata for a forum topic.

grm topic edit <supergroup_id> <topic_id> "<new_name>"
   Rename an existing forum topic.

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

grm file get [-o|--output <dir|file>] [-t|--topic <id>] <chat_id> <message_ids...>
   Download specific attachment files by message ID.

grm file download-all [-o|--output <dir>] [-t|--topic <id>] [-n|--limit <N>] [--type photo|video|doc|audio|all] <chat_id>
   Scan and bulk download attachment media files from a chat or forum topic.

   -o, --output <dir>
      Destination directory path.

   --type <type>
      Filter media type: ``photo``, ``video``, ``doc``, ``audio``, or ``all``.

Examples
========

1. List active chats in table format:

   .. code-block:: bash

      grm chat ls

2. Create a new forum topic in supergroup:

   .. code-block:: bash

      grm topic create -1003750297693 "DevOps Operations"

3. Send document attachment to topic:

   .. code-block:: bash

      grm msg send -t 2 -a /var/log/syslog -C "System logs" -1003750297693

4. Bulk download all photos from topic:

   .. code-block:: bash

      grm file download-all -o ~/Downloads -t 2 --type photo -1003750297693

Files
=====

~/.config/grm/grm.conf
   User configuration file containing API credentials and preferences.

~/.local/share/grm/tdlib/
   TDLib persistent database and session cache directory.

See Also
========

**rst2man**\(1), **sphinx-build**\(1), **tdlib**\(7)
