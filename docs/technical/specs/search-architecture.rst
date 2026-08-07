=======================================================
[TECH-007] Universal Search Architecture Specification
=======================================================

Architecture Overview
=====================

The Universal Cross-Domain Search module (`src/cmd_search.cpp`) routes query requests to specific TDLib endpoints based on domain selection.

TDLib API Endpoints
===================

1. Chat Search (`searchChats` & `searchPublicChats`)
---------------------------------------------------

* Request: `{"query": "<text>", "limit": <count>}`
* Returns `chats` object containing `chat_ids` vector.

2. Message Search (`searchMessages` & `searchChatMessages`)
----------------------------------------------------------

* Global: `searchMessages` (`{"query": "<text>", "limit": <count>}`)
* Per-Chat: `searchChatMessages` (`{"chat_id": <id>, "query": "<text>", "limit": <count>}`)
* Returns `messages` object containing `messages` array.

3. User Search (`searchContacts` & `searchUserByUsername`)
---------------------------------------------------------

* `searchContacts` (`{"query": "<text>", "limit": <count>}`)
* `searchUserByUsername` (`{"username": "<handle>"}`)
* Returns `users` or `user` object.

4. Universal Multi-Domain Aggregator
------------------------------------

Executing `grm search "<query>"` triggers parallel or sequential TDLib requests across chat, user, and message domains, aggregating top matches into a `SearchSummary` structure.

Data Structures
===============

```cpp
struct SearchSummary {
  std::vector<ChatSummary> chats;
  std::vector<UserSummary> users;
  std::vector<MessageSummary> messages;
};
```
