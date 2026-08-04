=====================================================
Functional Sub-Spec: Authentication & Chat Management
=====================================================

Requirements Traceability
=========================

* **[FUNC-001]**: User Authentication
* **[FUNC-002]**: Chat & Group CRUD Operations

Detailed Functional Descriptions
================================

[FUNC-001] User Authentication
------------------------------
The system must support interactive terminal-based authentication:

1. Prompt for international phone number format (e.g., ``+12025550123``) or accept via ``-p|--phone``.
2. Validate Telegram login code via SMS or app notification or accept via ``-k|--code``.
3. Prompt for Telegram 2FA cloud password with terminal echo disabled when 2FA is active.
4. Persist encrypted local session state inside ``~/.config/grm/tdlib_db/``.

[FUNC-002] Chat & Group CRUD Operations
---------------------------------------
The system must provide full CRUD control over private chats, basic groups, supergroups, and channels:

1. **Create Chat / Group / Channel**:
   - `grm chat create group [--private|--public] "<title>"`: Create a new basic group or supergroup.
   - `grm chat create channel [--private|--public] "<title>" ["<description>"]`: Create a new public or private broadcasting channel.

2. **Read Chat List & Detailed Info**:
   - `grm chat ls [-n|--limit <N>]`: Retrieve active chats sorted by activity.
   - `grm chat info <chat_id>`: Display comprehensive chat metadata (Title, Type, Member Count, Description, Invite Link, Permissions).

3. **Update Chat Settings**:
   - `grm chat set-title <chat_id> "<new_title>"`: Update chat or channel title.
   - `grm chat set-desc <chat_id> "<description>"`: Update chat or channel description text.
   - `grm chat pin <chat_id> <message_id>`: Pin a specific message to top of chat.
   - `grm chat unpin <chat_id> [<message_id>]`: Unpin message or clear all pinned messages.

4. **Delete / Leave Chat**:
   - `grm chat delete <chat_id>`: Leave basic group/supergroup or delete private chat history.
