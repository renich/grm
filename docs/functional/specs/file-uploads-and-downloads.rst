=========================================================
Functional Sub-Spec: File Upload & Attachment Download Engine
=========================================================

Requirements Traceability
=========================

* **[FUNC-007]**: File & Media Upload Dispatch
* **[FUNC-008]**: File & Media Attachment Download Engine

Detailed Functional Descriptions
================================

[FUNC-007] File & Media Upload Dispatch
---------------------------------------
The system must support uploading local files and media attachments directly to Telegram chats, supergroups, channels, and forum topics:

1. Accept target ``chat_id``, optional message text / caption, repeatable ``-a, --attach <file>`` flags, optional ``-m, --media`` mode flag, and optional ``-t, --topic <id>`` thread ID.
2. Verify local file existence and readability before initiating TDLib upload.
3. Support documents, photos, audio, and generic binary attachments using TDLib's ``inputFileLocal`` wrapper with ``inputMessageDocument`` or ``inputMessagePhoto`` payloads.
4. Display upload progress or completion confirmation upon successful dispatch.

[FUNC-008] File & Media Attachment Download Engine
--------------------------------------------------
The system must support downloading attachments (documents, photos, videos, audio) from chats and forum topics:

1. **Single / Selective File Download**:
   - `grm file get [-o|--output <dir|file>] [-t|--topic <id>] <chat_id> <message_ids...>`: Download file attachment(s) from specified message ID(s) to a target local path or directory (defaults to current directory).

2. **Bulk / Directory File Download**:
   - `grm file download-all [-o|--output <dir>] [-t|--topic <id>] [-n|--limit <N>] [--type photo|video|doc|audio|all] <chat_id>`: Batch download all attachments from a chat or specific forum topic matching file type filters.
