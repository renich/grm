====================================
Story & Emoji Status Management Spec
====================================

:Author: Rénich Bon Ćirić
:Status: Approved
:Type: Functional Specification
:Date: 2026-08-23

Overview
========

This specification defines the functional requirements for posting Telegram Stories (photos, videos, captions, privacy, and duration) and managing custom Telegram Emoji Statuses via the **grm** CLI.

Functional Requirements
=======================

1. Story Publishing (``grm story post``)
----------------------------------------

* **Media Types**: Supports local photo images (``--photo <path>``) or videos (``--video <path>``).
* **Captions**: Accepts optional Markdown-formatted caption text (``--caption "text"``).
* **Privacy Controls**: Supports privacy scope (``--privacy everyone|contacts|close_friends``; default: ``everyone``).
* **Active Duration**: Configurable story expiration (``--period <6h|12h|24h|48h>``; default: ``24h`` / 86400s).
* **Chat Target**: Supports publishing on behalf of personal user account or specified channel/supergroup (``--chat <id>``).
* **Pinned State**: Supports keeping the story on the chat profile/page after expiration (``--pinned``).
* **Content Protection**: Prevents story forwarding and saving (``--protect``).

2. Story Inspection & Deletion (``grm story ls`` & ``grm story delete``)
------------------------------------------------------------------------

* **Listing**: Lists active stories for personal account or target chat (``grm story ls [--chat <id>] [--limit <N>]``).
* **Deletion**: Deletes a published story by identifier (``grm story delete --story-id <id> [--chat <id>]``).

3. Custom Emoji Status Management (``grm status set`` & ``grm status clear``)
-----------------------------------------------------------------------------

* **Setting Status**: Sets a custom Telegram emoji badge (``grm status set --emoji <custom_emoji_id> [--duration <time>] [--chat <id>]``).
* **Duration Parsing**: Supports human duration strings like ``30m``, ``1h``, ``2d``, ``1w``, or raw seconds.
* **Clearing Status**: Resets the active emoji status to default (``grm status clear [--chat <id>]``).
