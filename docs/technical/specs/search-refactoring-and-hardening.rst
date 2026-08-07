===============================================================
[TECH-008] Universal Search Refactoring & Hardening Specification
===============================================================

Overview
========

This technical specification details the architectural hardening, safety protocols, DRY refactoring, and multi-type attachment search enhancements for the **grm** Universal Cross-Domain Search Engine (``src/cmd_search.cpp``).

These refactorings address edge cases identified during architectural review and prepare the module for implementation on the dedicated feature/refactor branch ``refactor/search-engine-hardening``.

Architectural Weaknesses Identified
====================================

1. **JSON Syntax Breakage via Unescaped Query Strings**
   Direct string formatting using ``std::format(R"({{"query": "{}"}})", query)`` fails when ``query`` contains double quotes (``"``), backslashes (``\``), or control characters.

2. **Duplicated Candidate Resolution Loops (DRY Violation)**
   The handlers ``cmd_search_chats``, ``cmd_search_supergroups``, and universal ``cmd_search`` duplicate identical logic for:
   * Querying ``searchChats`` and ``searchPublicChats``.
   * Calling ``ensure_chat_loaded``.
   * Sending ``getChat`` and parsing chat metadata into ``fmt::ChatItem``.

3. **Single-Type Media Scope in File Search**
   Searching file attachments currently hardcodes ``searchMessagesFilterDocument``, omitting photo, video, audio, and general media attachments.

Technical Refactoring Requirements
==================================

1. Mandatory JSON String Escaping (Injection Protection)
--------------------------------------------------------

All JSON payloads constructed for TDLib search requests MUST wrap query strings using ``escape_json_string()`` defined in ``include/grm/json_utils.hpp``.

.. code-block:: cpp

   const std::string escaped_query = escape_json_string(query);
   const std::string req = std::format(
       R"({{"query": "{}", "limit": {}}})",
       escaped_query, limit);

Target RPC Requests:
* ``searchChats``
* ``searchPublicChats``
* ``searchContacts``
* ``searchMessages``
* ``searchChatMessages``

2. DRY Chat Item Resolver Helper Function
-----------------------------------------

Consolidate chat metadata fetching and type classification into a single private helper method in ``include/grm/app.hpp`` and ``src/cmd_search.cpp``:

.. code-block:: cpp

   struct ResolvedChatItems {
     std::vector<fmt::ChatItem> chats;
     std::vector<fmt::ChatItem> supergroups;
   };

   ResolvedChatItems resolve_chat_candidates(
       const std::vector<int64_t> &chat_ids,
       int limit);

Responsibilities:
* Loop through candidate ``chat_ids``.
* Execute ``ensure_chat_loaded(id)`` for each candidate.
* Fetch ``getChat`` and classify into ``Basic Group``, ``Supergroup``, ``Forum Supergroup``, ``Channel``, or ``Private Chat``.
* Partition into ``chats`` and ``supergroups`` vectors.

3. Multi-Type File Attachment Search (``--type`` Option)
-------------------------------------------------------

Enhance ``grm search files <query> [options]`` to accept a ``--type <doc|photo|video|audio|all>`` option flag.

Supported Filters:
* ``doc`` (default): ``searchMessagesFilterDocument``
* ``photo``: ``searchMessagesFilterPhoto``
* ``video``: ``searchMessagesFilterVideo``
* ``audio``: ``searchMessagesFilterAudio``
* ``all``: Aggregates matches across document, photo, video, and audio filters.

Command Specification Update in ``App::get_search_spec()``:

.. code-block:: cpp

   {"files", "<query> [options]", "Search file and media attachments",
    {{"-t", "--type", "<doc|photo|video|audio|all>", "Filter attachment type (default: doc)", {"doc", "photo", "video", "audio", "all"}},
     {"-n", "--limit", "<count>", "Maximum search results (default: 20)", {}},
     {"-v", "--verbose", "", "Show verbose metadata", {}}}}

Execution & Verification Protocol
==================================

Branch Strategy
---------------

1. Checkout refactor branch: ``git checkout -b refactor/search-engine-hardening``
2. Implement ``escape_json_string`` wrapping across all RPC invocations in ``src/cmd_search.cpp``.
3. Refactor candidate chat resolution using ``resolve_chat_candidates()``.
4. Implement ``grm search files --type`` flag handling.
5. Update CTest suite in ``tests/test_search.cpp``.
6. Run ``make check`` (verify 100% CTest pass across all 16 test binaries).
7. Run ``make doc-check`` (verify zero rstcheck errors).
8. Execute live account verification against Telegram account.
9. Commit using Conventional Commits trailers and push to remotes.
