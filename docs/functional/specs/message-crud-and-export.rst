=================================================
Functional Sub-Spec: Message CRUD & Export Engine
=================================================

Requirements Traceability
=========================

* **[FUNC-003]**: Message CRUD Operations
* **[FUNC-006]**: Message Export Engine

Detailed Functional Descriptions
================================

[FUNC-003] Message CRUD Operations
----------------------------------
The system must support full CRUD management for messages in chats and forum topics:

1. **Create / Send Message & Attachments**:
   - `grm msg send [-a|--attach <file>] [-m|--media] [-C|--caption "<text>"] [-t|--topic <id>] <chat_id> ["<message>"]`: Send text message or upload single/multi-file document/media attachments.

2. **Read / List / Search / Info**:
   - `grm msg ls [-t|--topic <id>] [-n|--limit <N>] <chat_id>`: List recent messages from chat or topic.
   - `grm msg info <chat_id> <message_id>`: View detailed metadata for a single message (ID, Sender, Timestamp, Edit State, Views/Reactions).
   - `grm msg search [-t|--topic <id>] [-q|--query "<pattern>"] [-n|--limit <N>] <chat_id>`: Search message stream using query regex pattern filter.

3. **Update / Edit Message**:
   - `grm msg edit [-t|--topic <id>] <chat_id> <message_id> "<new_text>"`: Edit existing text content of an sent message.

4. **Delete Message**:
   - `grm msg delete [--for-everyone] <chat_id> <message_ids...>`: Delete one or multiple messages by ID (for self or for all chat members).

[FUNC-006] Message Export Engine
--------------------------------
The system must export chat and topic histories to structured files:

1. `grm msg export [-f|--format csv|json] [-o|--output <file>] [-t|--topic <id>] [-n|--limit <N>] <chat_id>`: Extract up to N messages and write to JSON or CSV file.
