===
grm
===

--------------------------------------------
Group & Telegram Manager CLI (C++23 / TDLib)
--------------------------------------------

:Authors: Rénich Bon Ćirić <renich@evalinux.com> & Antigravity AI (Google DeepMind)
:Date: 2026-08-04

:Manual section: 1
:Manual group: User Commands


SYNOPSIS
========

**grm** [*GLOBAL_OPTIONS*] *COMMAND* [*SUBCOMMAND*] [*OPTIONS*] [*ARGS*]

DESCRIPTION
===========

**grm** is a high-performance command-line interface for managing Telegram chats, supergroups, channels, forum topics, messages, and media files. Built with modern C++23 and TDLib, **grm** provides fast, offline-capable asynchronous operation with POSIX/GNU flag parsing and structured output formatting (tables, Markdown, JSON, CSV).

GLOBAL OPTIONS
==============

-h, --help
   Display command help message and exit.

-V, --version
   Output version and build information.

-v, --verbose
   Enable verbose TDLib logging.

-d, --debug
   Enable low-level debug tracing and JSON payload dumps.

-q, --quiet
   Suppress non-error messages.

-c, --config *FILE*
   Path to config file (default: ``~/.config/grm/grm.conf``).

-F, --format *FMT*
   Output format: ``human``, ``markdown``, ``json``, or ``plain``.

--color *MODE*
   Color mode: ``auto``, ``always``, or ``never``.

COMMANDS
========

login
-----

grm login [-p *PHONE*] [-k *CODE*]
   Authenticate session with Telegram servers.

chat
----

grm chat ls
   List active conversations, groups, channels.

grm chat create <group|channel> [--private|--public] "*TITLE*"
   Create group or channel.

grm chat info *CHAT_ID*
   Display chat metadata in JSON format.

grm chat set-title *CHAT_ID* "*TITLE*"
   Change chat title.

grm chat set-desc *CHAT_ID* "*DESCRIPTION*"
   Change supergroup/channel description.

grm chat pin *CHAT_ID* *MESSAGE_ID*
   Pin message in chat.

grm chat unpin *CHAT_ID* [*MESSAGE_ID*]
   Unpin message.

grm chat delete *CHAT_ID*
   Delete chat history or leave group.

msg
---

grm msg ls [-n *LIMIT*] [-t *TOPIC_ID*] *CHAT_ID*
   List chat or forum topic history.

grm msg export [-f csv|json] [-o *FILE*] [-t *TOPIC_ID*] *CHAT_ID*
   Export chat history to file.

grm msg search [-q "*QUERY*"] [-n *LIMIT*] [-t *TOPIC_ID*] *CHAT_ID*
   Search chat history.

grm msg send [-a *FILE*] [-m] [-C "*CAPTION*"] [-t *TOPIC_ID*] *CHAT_ID* ["*MESSAGE*"]
   Send message or file attachment.

grm msg info *CHAT_ID* *MESSAGE_ID*
   View message metadata.

grm msg edit [-t *TOPIC_ID*] *CHAT_ID* *MESSAGE_ID* "*TEXT*"
   Edit text message.

grm msg delete [--for-everyone] *CHAT_ID* *MESSAGE_IDS...*
   Delete messages.

topic
-----

grm topic ls *SUPERGROUP_ID*
   List active forum topics.

grm topic create *SUPERGROUP_ID* "*TOPIC_NAME*"
   Create new forum topic.

grm topic info *SUPERGROUP_ID* *TOPIC_ID*
   View topic metadata.

grm topic edit *SUPERGROUP_ID* *TOPIC_ID* "*NEW_NAME*"
   Rename topic.

grm topic close *SUPERGROUP_ID* *TOPIC_ID*
   Close topic.

grm topic reopen *SUPERGROUP_ID* *TOPIC_ID*
   Reopen topic.

grm topic pin *SUPERGROUP_ID* *TOPIC_ID*
   Pin topic.

grm topic unpin *SUPERGROUP_ID* *TOPIC_ID*
   Unpin topic.

grm topic delete *SUPERGROUP_ID* *TOPIC_ID*
   Delete topic.

file
----

grm file get [-o *OUTPUT*] [-t *TOPIC_ID*] *CHAT_ID* *MESSAGE_IDS...*
   Download attachments by message ID.

grm file download-all [-o *DIR*] [-t *TOPIC_ID*] [-n *LIMIT*] [--type photo|video|doc|audio|all] *CHAT_ID*
   Bulk download media attachments.

EXIT STATUS
===========

0
   Success.

1
   General failure or TDLib API error.

2
   Invalid CLI arguments or usage syntax error.

FILES
=====

~/.config/grm/grm.conf
   Configuration file.

~/.local/share/grm/tdlib/
   TDLib database directory.

REPORTING BUGS
==============

Report bugs to <renich@evalinux.com> or via GitLab issue tracker at <https://gitlab.com/renich/grm>.

SEE ALSO
========

**rst2man**\(1), **tdlib**\(7)
