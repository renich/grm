=============================================================
Functional Sub-Spec: Supergroup Forum Topics CRUD Operations
=============================================================

Requirements Traceability
=========================

* **[FUNC-009]**: Supergroup Forum Topics CRUD Operations

Detailed Functional Descriptions
================================

[FUNC-009] Supergroup Forum Topics CRUD Operations
--------------------------------------------------
The system must provide complete CRUD lifecycle management for Telegram Supergroup Forum Topics (threads):

1. **Create Forum Topic**:
   - `grm topic create <supergroup_id> "<topic_name>" [--icon-color <color>] [--icon-emoji <id>]`: Create a new forum topic thread in a supergroup.

2. **Read / List / Info Forum Topics**:
   - `grm topic ls [-n|--limit <N>] <supergroup_id>`: List active forum topics in a supergroup (ID, Name, Message Count, Creation Date).
   - `grm topic info <supergroup_id> <topic_id>`: Display detailed topic metadata (ID, Title, Icon Emoji, Creator, Status, Message Count).

3. **Update / Edit / Toggle Forum Topics**:
   - `grm topic edit <supergroup_id> <topic_id> [--name "<new_name>"] [--icon-emoji <id>]`: Rename or change icon for an existing topic.
   - `grm topic close <supergroup_id> <topic_id>`: Close a forum topic thread (preventing new messages).
   - `grm topic reopen <supergroup_id> <topic_id>`: Reopen a closed forum topic thread.
   - `grm topic pin <supergroup_id> <topic_id>` / `grm topic unpin <supergroup_id> <topic_id>`: Pin/unpin topic in supergroup topic list.

4. **Delete Forum Topic**:
   - `grm topic delete <supergroup_id> <topic_id>`: Permanently delete a forum topic thread and all its history.
