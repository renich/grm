=====================================================
Functional Sub-Spec: Authentication & Chat Management
=====================================================

Requirements Traceability
=========================

* **[FUNC-001]**: User Authentication
* **[FUNC-002]**: Chat Discovery
* **[FUNC-003]**: Message History Inspection
* **[FUNC-004]**: Birthday Data Extraction
* **[FUNC-005]**: Message Dispatch

Detailed Functional Descriptions
================================

[FUNC-001] User Authentication
------------------------------
The system must support interactive terminal-based authentication:

1. Prompt for international phone number format (e.g., ``+52XXXXXXXXXX``).
2. Validate Telegram login code via SMS or app notification.
3. Prompt for Telegram cloud password with terminal echo disabled (silent non-echoing entry) when enabled on the account.
4. Persist encrypted local session state inside ``~/.config/grm/tdlib_db/``.


[FUNC-002] Chat & Group Discovery
---------------------------------
The system must list all active chats (private messages, basic groups, supergroups, and channels):

1. Retrieve up to 100 active chats sorted by recent activity.
2. Display internal peer ID, chat type, and clean title string.
3. Support negative channel IDs (e.g., ``-1001789902965``) without peer access hash errors.

[FUNC-003] Message History Inspection
-------------------------------------
The system must retrieve message streams from any target peer:

1. Accept target ``chat_id`` and optional ``limit`` parameters.
2. Display message ID, sender identity, and message text content.

[FUNC-004] Birthday Data Extraction
-----------------------------------
The system must scan chat message histories to extract declared user birthdays:

1. Match commands (e.g., ``/setbirthday``, ``/micumple``) and date strings (``DD/MM/YYYY``, ``YYYY-MM-DD``).
2. Format extracted dates into ISO ``YYYY-MM-DD`` records.

[FUNC-005] Direct Text Message Dispatch
---------------------------------------
The system must send text messages to any chat ID:

1. Accept target ``chat_id`` and message string.
2. Deliver message via TDLib ``sendMessage`` API.
