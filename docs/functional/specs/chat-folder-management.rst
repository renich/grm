===================================================
Functional Specification: Chat Folder Management
===================================================

Overview
--------

This specification details the **Chat Folder Management** CLI subcommand suite (``grm folder``) and chat list folder filtering (``grm chat ls --folder <id>``) for **grm** (Requirement ID: ``[FUNC-013]``).

Functional Requirements
-----------------------

* **[FUNC-013.1] Chat Folder Listing**:
  * Command: ``grm folder ls`` (or ``grm folder list``).
  * Displays all configured chat folders/filters in the user's Telegram account including Folder ID, Title, Included entity categories (groups, channels, bots, contacts, non-contacts), Excluded status flags (muted, read, archived), Pinned chat count, and Included chat count.
  * Formats: Human TTY Table (with color badges), Markdown Table, and structured JSON (``grm -F json folder ls``).

* **[FUNC-013.2] Chat Folder Creation**:
  * Command: ``grm folder create <title> [options...]``
  * Creates a new chat folder with custom title and filter criteria.
  * Options:
    * ``--include-groups`` / ``-g``: Include group chats.
    * ``--include-channels`` / ``-c``: Include channels.
    * ``--include-bots`` / ``-b``: Include bots.
    * ``--include-contacts`` / ``-C``: Include contacts.
    * ``--include-non-contacts`` / ``-N``: Include non-contacts.
    * ``--exclude-muted``: Exclude muted chats.
    * ``--exclude-read``: Exclude read chats.
    * ``--exclude-archived``: Exclude archived chats.
    * ``--include-chats <id...>`` / ``-i <id>``: Include explicit chat IDs.
    * ``--pinned-chats <id...>`` / ``-p <id>``: Pin explicit chat IDs within folder.

* **[FUNC-013.3] Chat Folder Editing**:
  * Command: ``grm folder edit <folder_id> [options...]``
  * Modifies an existing chat folder's title, filter criteria, or explicit chat inclusions/exclusions.
  * Options:
    * ``--title <title>`` / ``-t <title>``: Update folder title.
    * ``--add-chat <id>`` / ``-a <id>``: Add a chat ID to included list.
    * ``--remove-chat <id>`` / ``-r <id>``: Remove a chat ID from included list.
    * ``--pin-chat <id>`` / ``-P <id>``: Pin a chat ID in folder.
    * ``--unpin-chat <id>`` / ``-U <id>``: Unpin a chat ID in folder.

* **[FUNC-013.4] Chat Folder Deletion**:
  * Command: ``grm folder delete <folder_id>``
  * Removes a chat folder filter by ID.

* **[FUNC-013.5] Chat Listing Folder Filter**:
  * Command: ``grm chat ls --folder <folder_id>`` (or ``-F <folder_id>``)
  * Restricts chat listing strictly to chats contained within the specified chat folder.
