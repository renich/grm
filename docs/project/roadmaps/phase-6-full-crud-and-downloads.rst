=======================================================
Phase 6 Roadmap: Full CRUD & File Download Engine
=======================================================

Objective
=========
Expand **grm** into a comprehensive, full-featured management CLI providing 100% CRUD coverage across all Telegram entities (Chats, Groups, Channels, Forum Topics, Messages) alongside a dedicated single and bulk File Download engine (`grm file get`, `grm file download-all`).

Module Breakdown & Milestones
=============================

Module 6.1: Chat & Group CRUD Extensions (`grm chat`)
-----------------------------------------------------

- [x] Task 6.1.1: Implement `grm chat create group [--private|--public] "<title>"`
- [x] Task 6.1.2: Implement `grm chat create channel [--private|--public] "<title>" ["<desc>"]`
- [x] Task 6.1.3: Implement `grm chat info <chat_id>` (Detailed metadata, permissions, member counts)
- [x] Task 6.1.4: Implement `grm chat set-title <chat_id> "<title>"` & `grm chat set-desc <chat_id> "<desc>"`
- [x] Task 6.1.5: Implement `grm chat pin <chat_id> <message_id>` & `grm chat unpin <chat_id> [<message_id>]`
- [x] Task 6.1.6: Implement `grm chat delete <chat_id>` (Leave group / delete history)

Module 6.2: Forum Topics CRUD Extensions (`grm topic`)
------------------------------------------------------

- [x] Task 6.2.1: Implement `grm topic create <supergroup_id> "<name>"`
- [x] Task 6.2.2: Implement `grm topic info <supergroup_id> <topic_id>`
- [x] Task 6.2.3: Implement `grm topic edit <supergroup_id> <topic_id> [--name "<name>"]`
- [x] Task 6.2.4: Implement `grm topic close <supergroup_id> <topic_id>` & `grm topic reopen <supergroup_id> <topic_id>`
- [x] Task 6.2.5: Implement `grm topic pin <supergroup_id> <topic_id>` & `grm topic unpin <supergroup_id> <topic_id>`
- [x] Task 6.2.6: Implement `grm topic delete <supergroup_id> <topic_id>`

Module 6.3: Message CRUD Extensions (`grm msg`)
-----------------------------------------------

- [x] Task 6.3.1: Implement `grm msg info <chat_id> <message_id>` (View details, reactions, edits)
- [x] Task 6.3.2: Implement `grm msg edit [-t|--topic <id>] <chat_id> <message_id> "<new_text>"`
- [x] Task 6.3.3: Implement `grm msg delete [--for-everyone] <chat_id> <message_ids...>`

Module 6.4: Single & Bulk File Download Engine (`grm file`)
-----------------------------------------------------------

- [x] Task 6.4.1: Implement `grm file get [-o|--output <dir|file>] [-t|--topic <id>] <chat_id> <message_ids...>` (Download specific file attachments)
- [x] Task 6.4.2: Implement `grm file download-all [-o|--output <dir>] [-t|--topic <id>] [-n|--limit <N>] [--type photo|video|doc|audio|all] <chat_id>` (Bulk download attachments from chat/topic)
