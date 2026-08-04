===========================================================
Functional Sub-Spec: File Uploads & Forum Topics Management
===========================================================

Requirements Traceability
=========================

* **[FUNC-007]**: File & Media Upload Dispatch
* **[FUNC-008]**: Supergroup Forum Topics Discovery & Management

Detailed Functional Descriptions
================================

[FUNC-007] File & Media Upload Dispatch
---------------------------------------
The system must support uploading local files and media attachments directly to Telegram chats, supergroups, and channels:

1. Accept target ``chat_id``, local ``file_path``, optional ``--caption`` text, and optional ``--topic`` thread ID.
2. Verify local file existence and readability before initiating TDLib upload.
3. Support documents, photos, audio, and generic binary attachments using TDLib's ``inputFileLocal`` wrapper and ``inputMessageDocument`` payload.
4. Display upload progress or completion confirmation upon successful dispatch.

[FUNC-008] Supergroup Forum Topics Discovery & Management
---------------------------------------------------------
The system must support Telegram Supergroup Forum Topics (message threads):

1. **List Topics**: Retrieve and display active forum topics within a supergroup via ``grm topic ls <supergroup_id>``, showing Topic ID, Name, Icon Emoji/Color, and Creation Date.
2. **Topic Message Inspection**: Allow filtering message inspection by topic thread ID using ``grm msg ls <supergroup_id> [limit] --topic <topic_id>``.
3. **Topic Message & File Dispatch**: Allow targeting specific topic thread IDs when dispatching text messages or uploading files (``--topic <topic_id>``).
