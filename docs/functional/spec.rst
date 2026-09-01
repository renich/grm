============================
grm Functional Specification
============================

.. toctree::
   :maxdepth: 2

   specs/auth-and-chat-management
   specs/message-crud-and-export
   specs/file-uploads
   specs/file-uploads-and-downloads
   specs/forum-topics
   specs/forum-topics-crud
   specs/rich-text-emojis-and-branding
   specs/chat-folder-management
   specs/universal-cross-domain-search
   specs/story-and-status-management
   specs/cli-options-and-output
   specs/cli-subcommand-help-and-autocompletion
   specs/output-formatting-and-llm-modes

Overview
========

The **grm** (Group & Telegram Manager CLI) utility provides a fast, dependable command-line interface for full CRUD management across Telegram entities: Chats, Private/Public Groups, Channels, Forum Topics, Messages, File Downloads/Uploads, Chat Folder Organization, Universal Cross-Domain Search, Telegram Stories, and Custom Emoji Statuses.

Core Functional Requirements
============================

* **[FUNC-001]**: Interactive & Non-Interactive Telegram User Authentication via TDLib.
* **[FUNC-002]**: Full Chat & Group CRUD (Create Groups/Channels, Read Info/List, Update Title/Desc/Pin, Delete/Leave).
* **[FUNC-003]**: Full Message CRUD (Create/Send, Read/List/Search/Info, Update/Edit, Delete for self/everyone).
* **[FUNC-006]**: Message History Export Engine (CSV and JSON formats).
* **[FUNC-007]**: File & Media Upload Dispatch (Documents, Images, Video, Audio).
* **[FUNC-008]**: File & Media Attachment Download Engine (Single & Bulk Download per Chat/Topic).
* **[FUNC-009]**: Supergroup Forum Topics CRUD (Create Topic, Read List/Info, Update Edit/Close/Reopen/Pin, Delete Topic).
* **[FUNC-010]**: Telegram Rich Text Entity Formatting (Markdown V2 / HTML via TDLib ``parseTextEntities``).
* **[FUNC-011]**: Supergroup Forum Topic Custom Emoji Icons (``-e | --emoji``).
* **[FUNC-012]**: Project Branding & Visual Logo Assets (Crystal Shard / Facet Visual Identity).
* **[FUNC-013]**: Chat Folder Management (Create/List/Edit/Delete chat folders and filter chat listings by folder).
* **[FUNC-014]**: Universal Cross-Domain Search (Unified server-side and local search across chats, messages, and contacts).
* **[FUNC-015]**: Telegram Stories & Custom Emoji Status Management (Full story lifecycle, interactive sticker overlays, viewers analytics, profile pinning, reactions, privacy management, stealth mode, and custom status badges).

