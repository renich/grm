==========================
grm Technical Architecture
==========================

.. toctree::
   :maxdepth: 2

   specs/tdlib-client-architecture
   specs/file-uploader-architecture
   specs/forum-topics-architecture
   specs/chat-folder-architecture
   specs/search-architecture
   specs/search-refactoring-and-hardening

Architectural Blueprint
=======================

The **grm** tool is implemented as a modern C++23 native application linking against Telegram's official C++ engine **TDLib** (``libtdjson.so``) and ``json-c``.

System Design Principles
========================

1. **Modern C++23**: Enforces ``std::expected`` for error handling, smart pointers for RAII, and explicit type conversions.
2. **Asynchronous Receiver Thread**: Dedicated worker thread invoking ``td_receive`` to process incoming JSON events without blocking the main CLI.
3. **Promise-Based Synchronization**: Requests sent via ``td_send`` carry a unique ``@extra`` request ID mapped to a ``std::promise<JsonValue>`` for synchronous CLI commands.
4. **Zero Peer Hash Bugs**: Relies on TDLib's internal state machine for automatic ``AccessHash`` resolution across all supergroups and channels.

Technical Traceability Matrix
=============================

+-------------------+----------------------------+-----------------------+
| Technical Spec ID | Component                  | Fulfills Requirement  |
+===================+============================+=======================+
| ``[TECH-001]``    | ``grm::TdClient``          | ``[FUNC-001]``,       |
|                   |                            | ``[FUNC-002]``        |
+-------------------+----------------------------+-----------------------+
| ``[TECH-002]``    | ``grm::JsonValue``         | ``[FUNC-003]``,       |
|                   |                            | ``[FUNC-004]``        |
+-------------------+----------------------------+-----------------------+
| ``[TECH-003]``    | ``grm::App`` Controller    | ``[FUNC-005]``,       |
|                   |                            | ``[FUNC-006]``        |
+-------------------+----------------------------+-----------------------+
| ``[TECH-004]``    | File Uploader Engine       | ``[FUNC-007]``        |
+-------------------+----------------------------+-----------------------+
| ``[TECH-005]``    | Forum Topics Engine        | ``[FUNC-008]``        |
+-------------------+----------------------------+-----------------------+
| ``[TECH-006]``    | Chat Folder Engine         | ``[FUNC-013]``        |
+-------------------+----------------------------+-----------------------+
| ``[TECH-007]``    | Search Engine              | ``[FUNC-014]``        |
+-------------------+----------------------------+-----------------------+
| ``[TECH-008]``    | Search Hardening Engine    | ``[FUNC-014]``        |
+-------------------+----------------------------+-----------------------+

