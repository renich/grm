===
grm
===

-------------------------------
Group & Telegram Manager CLI
-------------------------------

:Author: Rénich Bon Ćirić <renich@evalinux.com>
:Date: 2026-08-05
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
   Display command or subcommand help message and exit.

-V, --version
   Output version and build information.

-v, --verbose
   Enable verbose TDLib logging.

-d, --debug
   Enable low-level debug tracing and JSON payload dumps.

-q, --quiet
   Suppress non-error messages.

-c, --config *FILE*
   Specify custom path to configuration file (default: ``~/.config/grm/grm.conf``).

-T, --test-dc
   Connect to Telegram Test Data Center (DC) environment.

-F, --format *FMT*
   Set output format: ``human``, ``markdown``, ``json``, or ``plain`` (default: ``auto``).

-N, --name-format *FMT*
   Set sender name display format: ``username`` (default, e.g. ``@username``) or ``fullname`` (e.g. ``First Last``).

-u, --username
   Shortcut to set sender name format to ``username``.

--full-name
   Shortcut to set sender name format to ``fullname``.

--color *MODE*
   Control ANSI color output: ``auto``, ``always``, or ``never`` (or ``--no-color``).

COMMANDS
========

login
-----

grm login [-p *PHONE*] [-k *CODE*] [-q]
   Authenticate Telegram session via interactive terminal phone verification or browser QR code.

   -p, --phone *PHONE*
      Pre-fill international phone number (e.g. ``+523330000000``).

   -k, --code *CODE*
      Pre-fill authentication code for non-interactive logins.

   -q, --qr
      Authenticate via QR code by generating ``/tmp/grm-login-qr.html`` and opening it via ``xdg-open``.

logout
------

grm logout
   Log out from Telegram, terminate the active TDLib session, and clear local session state.

chat
----

grm chat ls [-n *LIMIT*] [-S *SINCE*] [-f *PATTERN*]
   List active conversations, groups, channels, and private chats.

   -n, --limit *N*
      Maximum number of chats to display (default: 100).

   -S, --since *SINCE*
      Filter chats active since natural duration (e.g. ``"1 day ago"``, ``"3 days ago"``, ``"2h"``), ISO date, or timestamp.

   -f, --filter *PATTERN*
      Filter chats by title, type, or ID (case-insensitive substring or regex). Can be passed multiple times.

grm chat create group [--private|--public] "*TITLE*"
   Create a new basic group or supergroup.

grm chat create channel [--private|--public] "*TITLE*" ["*DESCRIPTION*"]
   Create a new broadcast channel.

grm chat info *CHAT_ID*
   Display detailed chat or supergroup metadata.

grm chat set-title *CHAT_ID* "*TITLE*"
   Update group or channel title.

grm chat set-desc *CHAT_ID* "*DESCRIPTION*"
   Update the description of a supergroup or channel.

grm chat pin *CHAT_ID*
   Pin a chat or group to the top of the chat list.

grm chat unpin *CHAT_ID*
   Unpin a chat or group from the chat list.

grm chat delete *CHAT_ID*
   Delete chat history or leave group.

Message Lifecycle
-----------------

grm msg ls [-n *LIMIT*] [-t *TOPIC_ID*] [-S *SINCE*] [-f *PATTERN*] [-r] *CHAT_ID*
   List messages from a chat or forum topic in chronological order (oldest first at top, newest at bottom).

   -n, --limit *N*
      Maximum number of messages to display (default: 20).

   -t, --topic *TOPIC_ID*
      Filter messages by forum topic ID.

   -S, --since *SINCE*
      Filter messages since natural human/systemd duration (e.g. ``"1 day ago"``, ``"3 days ago"``, ``"1 month ago"``, ``"2h"``, ``"yesterday"``), ISO date (``YYYY-MM-DD``), or timestamp.

   -f, --filter, --sender *PATTERN*
      Filter messages by sender username, display name, or handle (case-insensitive substring or regex). Can be passed multiple times (e.g. ``--filter="renichbon" --filter="k_brown"``) to match any specified sender.

   -r, --reverse
      Display messages in reverse-chronological order (newest first at top).

grm msg pin *CHAT_ID* *MESSAGE_ID*
   Pin a specific message in a chat or supergroup.

grm msg unpin *CHAT_ID* *MESSAGE_ID*
   Unpin a specific message in a chat or supergroup.

grm msg unpin-all *CHAT_ID*
   Unpin all pinned messages in a chat or supergroup.

grm msg export [-f csv|json] [-o *FILE*] [-t *TOPIC_ID*] *CHAT_ID*
   Export chat history to CSV or JSON file.

   -f, --format *FMT*
      Export format: ``csv`` or ``json``.

   -o, --output *FILE*
      Output target file path.

   -t, --topic *TOPIC_ID*
      Target specific forum topic ID.

grm msg search [-q "*QUERY*"] [-n *LIMIT*] [-t *TOPIC_ID*] [-S *SINCE*] [-f *PATTERN*] *CHAT_ID*
   Search message history using pattern or regex filter.

   -q, --query "*QUERY*"
      Regex search pattern.

   -n, --limit *N*
      Maximum matching messages to return.

   -t, --topic *TOPIC_ID*
      Target specific forum topic ID.

   -S, --since *SINCE*
      Filter messages since natural human/systemd duration (e.g. ``"1 day ago"``, ``"3 days ago"``, ``"1 month ago"``, ``"2h"``, ``"yesterday"``), ISO date (``YYYY-MM-DD``), or timestamp.

   -f, --filter, --sender *PATTERN*
      Filter messages by sender username, display name, or handle (case-insensitive substring or regex).

grm msg send [-a *FILE*] [-m] [-C "*CAPTION*"] [-t *TOPIC_ID*] *CHAT_ID* ["*MESSAGE*"]
   Send text message, document, or media attachment. Supports Telegram Rich Text Markdown V2 formatting.

   -a, --attach *FILE*
      Path to local file attachment (repeatable).

   -m, --media
      Send attachment as compressed media (photo/video/audio).

   -C, --caption "*CAPTION*"
      Caption text for file attachment.

   -t, --topic *TOPIC_ID*
      Target specific forum topic ID.

grm msg info *CHAT_ID* *MESSAGE_ID*
   Display detailed JSON payload for a message.

grm msg edit *CHAT_ID* *MESSAGE_ID* "*NEW_TEXT*"
   Edit text of an existing message.

grm msg delete [-e] *CHAT_ID* *MESSAGE_IDS...*
   Delete one or more messages by ID.

   -e, --for-everyone
      Delete messages for all chat participants.

Supergroup Topic Management
---------------------------

grm topic ls [-n *LIMIT*] [-S *SINCE*] [-f *PATTERN*] *CHAT_ID*
   List forum topics in a supergroup.

   -n, --limit *N*
      Maximum number of topics to display (default: 100).

   -S, --since *SINCE*
      Filter topics active since natural duration (e.g. ``"1 day ago"``, ``"3 days ago"``, ``"2h"``), ISO date, or timestamp.

   -f, --filter *PATTERN*
      Filter topics by name or thread ID (case-insensitive substring or regex). Can be passed multiple times.

grm topic create [-e *CUSTOM_EMOJI_ID*] [--icon-color *COLOR*] *SUPERGROUP_ID* "*TOPIC_NAME*"
   Create a new forum topic in a supergroup with optional custom emoji icon.

   -e, --emoji, --icon *ID*
      Custom Telegram emoji icon identifier.

   --icon-color *COLOR*
      RGB icon color hex integer (e.g. 0x6FB9F0).

grm topic info *SUPERGROUP_ID* *TOPIC_ID*
   View detailed metadata for a forum topic.

grm topic edit [-e *CUSTOM_EMOJI_ID*] *SUPERGROUP_ID* *TOPIC_ID* ["*NEW_NAME*"]
   Rename an existing forum topic or change its custom emoji icon.

   -e, --emoji, --icon *ID*
      Custom Telegram emoji icon identifier.

grm topic close *SUPERGROUP_ID* *TOPIC_ID*
   Close a forum topic.

grm topic reopen *SUPERGROUP_ID* *TOPIC_ID*
   Reopen a closed forum topic.

grm topic pin *SUPERGROUP_ID* *TOPIC_ID*
   Pin a forum topic.

grm topic unpin *SUPERGROUP_ID* *TOPIC_ID*
   Unpin a forum topic.

grm topic delete *SUPERGROUP_ID* *TOPIC_ID*
   Delete a forum topic and its message history.

file
----

grm file get [-A] [-o *OUTPUT*] [-t *TOPIC_ID*] [-n *LIMIT*] [--type photo|video|doc|audio|all] *CHAT_ID* [*MESSAGE_IDS...*]
   Download specific attachment files by message ID, or bulk download all attachments when -A, --all is specified.

   -A, --all
      Bulk download all attachment media files from chat or topic.

   -o, --output *OUTPUT*
      Output directory or destination file path.

   -t, --topic *TOPIC_ID*
      Target specific forum topic ID.

   -n, --limit *LIMIT*
      Maximum messages to scan (default: 100).

   --type *TYPE*
      Filter media type: ``photo``, ``video``, ``doc``, ``audio``, or ``all``.

EXIT STATUS
===========

0
   Success.

1
   General failure or TDLib API error.

2
   Invalid CLI arguments or usage syntax error.

PLANNED FEATURES & ROADMAP
==========================

The following feature modules represent the planned functional roadmap for **grm**:

1. Chat Folder Management (grm folder)
--------------------------------------
* ``grm folder ls``: List defined chat folders, included chat types, and pinned chats (TDLib ``getChatFolder``).
* ``grm folder create <title> [--include-groups] [--include-channels] [--include-chats <ids...>]``: Create custom chat folder (TDLib ``createChatFolder``).
* ``grm folder edit <folder_id> [--title <title>] [--add-chat <id>] [--remove-chat <id>]``: Edit folder configuration (TDLib ``editChatFolder``).
* ``grm folder delete <folder_id>``: Remove chat folder (TDLib ``deleteChatFolder``).

2. Universal Cross-Domain Search (grm search)
---------------------------------------------
* ``grm search chats <query>``: Global search across chats, channels, and supergroups (TDLib ``searchChats``, ``searchPublicChats``).
* ``grm search msgs <query> [-c <chat_id>] [-t <type>]``: Search message contents, senders, and captions (TDLib ``searchMessages``, ``searchChatMessages``).
* ``grm search users <query|phone|handle>``: Search user profiles and contacts (TDLib ``searchContacts``, ``searchUserByUsername``).

3. Media & File Filtering/CRUD by Type (grm file)
-------------------------------------------------
* ``grm file ls <chat_id> [--type photo|video|doc|audio|voice|url|all] [-n limit]``: List media attachments filtered by type (TDLib ``SearchMessagesFilter``).
* ``grm file download <chat_id> <file_id|msg_id> [-o <path>]``: Download specific document or media attachment.
* ``grm file download-all <chat_id> [--type photo|video|doc|audio|all] [-o <dir>]``: Bulk download matching media attachments.

4. Contact Management (grm contact)
-----------------------------------
* ``grm contact ls``: List saved contacts and online availability status (TDLib ``getContacts``).
* ``grm contact info <user_id|phone|handle>``: Inspect user profile details and bio (TDLib ``getUser``, ``getUserFullInfo``).
* ``grm contact add <phone> <first_name> [last_name] [--share-phone]``: Import or add new contact (TDLib ``addContact``, ``importContacts``).
* ``grm contact delete <user_id>``: Remove user from contacts (TDLib ``removeContacts``).

5. Silent & Scheduled Messages (grm msg)
----------------------------------------
* ``grm msg send <chat_id> "<message>" [--silent] [--schedule-at "<datetime|duration>"] [--send-when-online]``: Send silent notifications (TDLib ``disable_notification``), schedule delivery at date/time (TDLib ``messageSchedulingStateSendAtDate``), or deliver when online (TDLib ``messageSchedulingStateSendWhenOnline``).
* ``grm msg scheduled ls <chat_id>``: View pending scheduled messages.
* ``grm msg scheduled delete <chat_id> <message_id>``: Cancel pending scheduled message.

6. Audio CRUD & Media Processing Engine (grm audio)
---------------------------------------------------
* ``grm audio send <chat_id> <file_path> [--title "<title>"] [--performer "<performer>"] [--silent]``: Send audio files with metadata (TDLib ``inputMessageAudio``).
* ``grm audio voice <chat_id> <voice_ogg_path> [--convert] [--silent]``: Send OGG/Opus voice notes (TDLib ``inputMessageVoiceNote``).
* **SoX / libsox Engine Integration**: Native integration with **libsox** to probe audio metadata, convert audio formats to OGG/Opus voice notes, and calculate exact amplitude waveforms.
* ``grm audio info <file_path>``: Inspect audio duration, sample rate, channels, and codec via libsox.

7. Attachment Transmission: Inline Media vs. Raw Document
---------------------------------------------------------
* **Inline Media (``--photo``, ``--video``, ``--animation``)**: Sent as compressed visual media (TDLib ``inputMessagePhoto``, ``inputMessageVideo``) featuring inline previews and streaming optimization.
* **Uncompressed Document (``--file`` / ``--doc``)**: Sent as raw binary document attachments (TDLib ``inputMessageDocument``) preserving byte-for-byte fidelity without compression.

ENVIRONMENT
===========

GRM_FORMAT
   Default output format mode: ``human``, ``markdown``, ``json``, ``jsonl`` (or ``ndjson``), or ``plain``. Overrides default TTY format detection when set.

GRM_COLOR
   Default ANSI color mode: ``auto``, ``always``, or ``never``.

NO_COLOR
   Disables ANSI color output when present and non-empty.

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
