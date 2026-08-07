============================================================
[FUNC-014] Universal Cross-Domain Search Functional Specification
============================================================

Overview
========

The Universal Cross-Domain Search engine (`grm search`) provides unified, server-side and local search across all Telegram entity domains: chats, supergroups, broadcast channels, messages, users, and public profiles.

Requirements
============

1. Subcommand Hierarchy
------------------------

* `grm search chats <query> [-n limit]`: Searches public and private chats, supergroups, and broadcast channels.
* `grm search msgs <query> [-c <chat_id>] [-t <type>] [-n limit]`: Searches message history across all chats or scoped to a target chat.
* `grm search users <query|phone|handle> [-n limit]`: Searches contacts and public user profiles by name, handle, or phone number.
* `grm search "<query>" [-n limit]`: Aggregates matches across chats, users, and messages into a single grouped summary view.

2. Formatting & UX
------------------

* **Human TTY View**: Formats search results in clear tables with term highlighting where appropriate.
* **Markdown View**: Generates AI-parseable GitHub Flavored Markdown tables.
* **JSON / JSONL View**: Programmatic JSON objects with ISO-8601 timestamps and count metadata.

3. Options & Limits
-------------------

* `-n, --limit <count>`: Maximum number of search results to return per domain (default: 20).
* `-c, --chat <chat_id>`: Restricts message search to a specific chat.
* `-v, --verbose`: Includes extended metadata (e.g., chat IDs, message IDs, user handles).
