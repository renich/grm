============================
grm Functional Specification
============================

.. toctree::
   :maxdepth: 2

   specs/auth-and-chat-management

Overview
========

The **grm** (Group & Telegram Manager CLI) utility provides a fast, dependable terminal interface for inspecting Telegram chats, viewing message histories, extracting user metadata (such as birthday declarations), and dispatching messages.

Core Functional Requirements
============================

* **[FUNC-001]**: Interactive Telegram User & Bot Authentication via TDLib.
* **[FUNC-002]**: Chat, Channel & Supergroup Discovery across all datacenters.
* **[FUNC-003]**: Message History Inspection & Filtering for any peer ID.
* **[FUNC-004]**: Regex Birthday & Data Extraction from chat history.
* **[FUNC-005]**: Direct Text Message Dispatch to individual users or groups.
