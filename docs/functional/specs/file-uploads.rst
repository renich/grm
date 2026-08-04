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

1. Accept target ``chat_id``, local ``file_path``, optional ``--caption`` text, and optional ``--topic`` thread ID.
2. Verify local file existence and readability before initiating TDLib upload.
3. Support documents, photos, audio, and generic binary attachments using TDLib's ``inputFileLocal`` wrapper and ``inputMessageDocument`` payload.
4. Display upload progress or completion confirmation upon successful dispatch.
