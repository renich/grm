============================
grm Functional Specification
============================

.. toctree::
   :maxdepth: 2

   specs/auth-and-chat-management
   specs/message-crud-and-export
   specs/file-uploads-and-downloads
   specs/forum-topics-crud

Overview
========

The **grm** (Group & Telegram Manager CLI) utility provides a fast, dependable command-line interface for full CRUD management across Telegram entities: Chats, Private/Public Groups, Channels, Forum Topics, Messages, and File Downloads/Uploads.

Core Functional Requirements
============================

* **[FUNC-001]**: Interactive & Non-Interactive Telegram User Authentication via TDLib.
* **[FUNC-002]**: Full Chat & Group CRUD (Create Groups/Channels, Read Info/List, Update Title/Desc/Pin, Delete/Leave).
* **[FUNC-003]**: Full Message CRUD (Create/Send, Read/List/Search/Info, Update/Edit, Delete for self/everyone).
* **[FUNC-006]**: Message History Export Engine (CSV and JSON formats).
* **[FUNC-007]**: File & Media Upload Dispatch (Documents, Images, Video, Audio).
* **[FUNC-008]**: File & Media Attachment Download Engine (Single & Bulk Download per Chat/Topic).
* **[FUNC-009]**: Supergroup Forum Topics CRUD (Create Topic, Read List/Info, Update Edit/Close/Reopen/Pin, Delete Topic).
