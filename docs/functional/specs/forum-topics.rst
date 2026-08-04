======================================================
Functional Sub-Spec: Supergroup Forum Topics (Threads)
======================================================

Requirements Traceability
=========================

* **[FUNC-008]**: Supergroup Forum Topics Discovery & Management

Detailed Functional Descriptions
================================

[FUNC-008] Supergroup Forum Topics Discovery & Management
---------------------------------------------------------
The system must support Telegram Supergroup Forum Topics (message threads):

1. **List Topics**: Retrieve and display active forum topics within a supergroup via ``grm topic ls <supergroup_id>``, showing Topic ID, Name, Icon Emoji/Color, and Creation Date.
2. **Topic Message Inspection**: Allow filtering message inspection by topic thread ID using ``grm msg ls -t <topic_id> [-n|--limit <N>] <supergroup_id>``.
3. **Topic Message & File Dispatch**: Allow targeting specific topic thread IDs when dispatching text messages or uploading files (``-t|--topic <topic_id>``).

