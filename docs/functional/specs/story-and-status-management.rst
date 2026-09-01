====================================
Story & Emoji Status Management Spec
====================================

:Author: Rénich Bon Ćirić
:Status: Approved
:Type: Functional Specification
:Date: 2026-08-24

Overview
========

This specification defines the functional requirements for posting, managing, and inspecting Telegram Stories (photos, videos, captions, privacy, duration, interactive stickers, viewers analytics, profile pinning, reactions, and stealth mode) and managing custom Telegram Emoji Statuses via the **grm** CLI.

Functional Requirements
=======================

1. Story Publishing (``grm story post``)
----------------------------------------

* **Media Types**: Supports local photo images (``--photo <path>``) or videos (``--video <path>``).
* **Captions**: Accepts optional caption text with native clickable URLs and Markdown entity parsing (``--caption "text"``).
* **Interactive Stickers / Overlays**:

  * Clickable link sticker overlay (``-l``, ``--link <url>``).
  * Interactive suggested reaction button (``-r``, ``--reaction <emoji>``).

* **Privacy Controls**: Supports privacy scope (``--privacy everyone|contacts|close_friends``; default: ``everyone``).
* **Active Duration**: Configurable story expiration (``--period <6h|12h|24h|48h>``; default: ``24h`` / 86400s).
* **Chat Target**: Supports publishing on behalf of personal user account or specified channel/supergroup (``-C``, ``--chat <id>``).
* **Pinned State**: Supports keeping the story on the chat profile/page under "Posts" after expiration (``--pinned``).
* **Content Protection**: Prevents story forwarding and saving (``--protect``).

2. Story Editing & Lifecycle Management (``grm story edit``, ``delete``, ``info``, ``viewers``)
-----------------------------------------------------------------------------------------------

* **Editing**: Updates media content, caption, clickable links, or reaction buttons of an existing story (``grm story edit --story-id <id> [--photo <path>|--video <path>] [--caption <caption>] [-l|--link <url>] [-r|--reaction <emoji>] [--chat <id>]``).
* **Inspection**: Displays full story metadata, interaction counters (views, forwards, reactions), capabilities, and interactive sticker attachments (``grm story info --story-id <id> [--chat <id>]``).
* **Viewers Analytics**: Lists viewer interaction history, timestamps, and reaction emojis with query search and pagination limits (``grm story viewers --story-id <id> [-n|--limit <N>] [-q|--query <text>] [--chat <id>]``).
* **Listing**: Lists active stories, pinned profile posts, and archived stories for personal account or target chat (``grm story ls [--chat <id>] [--limit <N>] [-p|--pinned] [-a|--archived] [-A|--all]``).
* **Deletion**: Deletes a published story by identifier (``grm story delete --story-id <id> [--chat <id>]``).

3. Story Interactions, Privacy & Stealth (``grm story pin``, ``unpin``, ``react``, ``privacy``, ``stealth``)
------------------------------------------------------------------------------------------------------------------------

* **Profile Pinning**: Dynamically pins (``grm story pin --story-id <id>``) or unpins (``grm story unpin --story-id <id>``) active stories from the profile "Posts" tab.
* **Story Reactions**: Sets or removes emoji reactions on stories (``grm story react --story-id <id> --emoji <emoji>``).
* **Privacy Management**: Dynamically updates the visibility scope of a published story (``grm story privacy --story-id <id> --privacy <everyone|contacts|close_friends>``).
* **Stealth Mode**: Activates Telegram Premium 25-minute stealth mode to hide past and future story views (``grm story stealth``).

4. Custom Emoji Status Management (``grm status ls``, ``grm status set``, ``grm status clear``)
------------------------------------------------------------------------------------------------

* **Listing & Searching**: Discovers available default, recent, and custom sticker pack emoji statuses with query filtering (``grm status ls [-f|--filter <query>] [--recent] [--packs] [query]``).
* **Setting Status**: Sets a custom Telegram emoji badge (``grm status set --emoji <custom_emoji_id> [--duration <time>] [--chat <id>]``).
* **Duration Parsing**: Supports human duration strings like ``30m``, ``1h``, ``2d``, ``1w``, or raw seconds.
* **Clearing Status**: Resets the active emoji status to default (``grm status clear [--chat <id>]``).
