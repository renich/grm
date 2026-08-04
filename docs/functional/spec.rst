============================
grm Functional Specification
============================

.. toctree::
   :maxdepth: 2

   specs/auth-and-chat-management
   specs/file-uploads
   specs/forum-topics


Overview
========

The **grm** (Group & Telegram Manager CLI) utility provides a fast, dependable terminal interface for inspecting Telegram chats, viewing message histories, extracting user metadata (such as birthday declarations), exporting data, uploading files, and managing forum topics.

Core Functional Requirements
============================

* **[FUNC-001]**: Interactive Telegram User & Bot Authentication via TDLib.
* **[FUNC-002]**: Chat, Channel & Supergroup Discovery across all datacenters.
* **[FUNC-003]**: Message History Inspection & Filtering for any peer ID.
* **[FUNC-004]**: Regex Birthday & Data Extraction from chat history.
* **[FUNC-005]**: Direct Text Message Dispatch to individual users or groups.
* **[FUNC-006]**: Message History Export Engine (CSV and JSON formats).
* **[FUNC-007]**: File & Media Upload Dispatch (Documents, Images, Audio).
* **[FUNC-008]**: Supergroup Forum Topics Discovery & Thread-Scoped Dispatch.

