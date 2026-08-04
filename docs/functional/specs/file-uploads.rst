============================================
Functional Sub-Spec: File & Media Attachment
============================================

Requirements Traceability
=========================

* **[FUNC-007]**: File & Media Upload Dispatch

Detailed Functional Descriptions
================================

[FUNC-007] File & Media Upload Dispatch
---------------------------------------
The system must support uploading local files and media attachments directly to Telegram chats, supergroups, and channels:

1. Accept target ``chat_id``, optional message text / caption, repeatable ``-a, --attach <file>`` flags, optional ``-m, --media`` mode flag, and optional ``-t, --topic <id>`` thread ID.
2. Verify local file existence and readability before initiating TDLib upload.
3. Support documents, photos, audio, and generic binary attachments using TDLib's ``inputFileLocal`` wrapper with ``inputMessageDocument`` or media payloads.
4. Support single and multi-file attachments (repeatable ``-a, --attach`` options).
5. Display upload progress or completion confirmation upon successful dispatch.

