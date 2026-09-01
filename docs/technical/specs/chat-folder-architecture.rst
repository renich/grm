======================================================
Technical Architecture: Chat Folder Management Engine
======================================================

Overview
--------

This technical sub-specification details the TDLib JSON API integration patterns for Telegram chat folder management (Requirement ID: ``[TECH-006]``).

TDLib Method Mappings
---------------------

1. **Get Chat Folders (``getChatFolders``)**:

   * Payload: ``{"@type": "getChatFolders", "main_chat_list_position": 0}``
   * Response: ``chatFolders`` object containing array ``chat_folders`` of ``chatFolderInfo`` items (``id``, ``title``, ``icon``, ``color_id``).

2. **Get Detailed Chat Folder Settings (``getChatFolder``)**:

   * Payload: ``{"@type": "getChatFolder", "chat_folder_id": <id>}``
   * Response: ``chatFolder`` object containing title, icon, colors, and chat filter criteria.

3. **Create Chat Folder (``createChatFolder``)**:

   Payload:

   .. code-block:: json

      {
        "@type": "createChatFolder",
        "folder": {
          "@type": "chatFolder",
          "title": "<title>",
          "icon": {"@type": "chatFolderIcon", "name": "Custom"},
          "color_id": -1,
          "pinned_chat_ids": [],
          "included_chat_ids": [],
          "excluded_chat_ids": [],
          "exclude_muted": false,
          "exclude_read": false,
          "exclude_archived": false,
          "include_contacts": false,
          "include_non_contacts": false,
          "include_bots": false,
          "include_groups": true,
          "include_channels": true
        }
      }

   Response: ``chatFolderInfo`` object containing assigned integer ID.

4. **Edit Chat Folder (``editChatFolder``)**:

   Payload:

   .. code-block:: json

      {
        "@type": "editChatFolder",
        "chat_folder_id": 1,
        "folder": {
          "@type": "chatFolder",
          "title": "<updated_title>"
        }
      }

   Response: ``ok`` or ``chatFolderInfo``.

5. **Delete Chat Folder (``deleteChatFolder``)**:

   * Payload: ``{"@type": "deleteChatFolder", "chat_folder_id": <id>}``
   * Response: ``ok``.

6. **Filter Chat Listing by Folder (``getChats``)**:

   Payload:

   .. code-block:: json

      {
        "@type": "getChats",
        "chat_list": {
          "@type": "chatListFolder",
          "chat_folder_id": 1
        },
        "limit": 50
      }
