=============================================
Rich Text, Topic Custom Emojis & Branding Spec
=============================================

This specification covers Telegram Rich Text Entity parsing (Markdown V2/HTML), Supergroup Forum Topic Custom Emoji Icons, and Project Branding Assets.

Functional Requirements
=======================

[FUNC-010] Telegram Rich Text Entity Formatting
------------------------------------------------

- **Requirement**: ``grm msg send`` must parse user text using TDLib's ``parseTextEntities`` function with ``textParseModeMarkdown`` (version 2) or HTML.
- **Behavior**:
  - Text input containing Markdown entities (e.g., ``**bold**``, ``*italic*``, ``code``, ``[text](url)``) is transformed into TDLib ``formattedText`` JSON structure containing an ``entities`` array of ``textEntity`` objects (``textEntityTypeBold``, ``textEntityTypeItalic``, ``textEntityTypeCode``, ``textEntityTypeTextUrl``).
  - Preserves raw string fallbacks if entity parsing fails.

[FUNC-011] Supergroup Forum Topic Custom Emoji Icons
----------------------------------------------------

- **Requirement**: ``grm topic create`` and ``grm topic edit`` must support custom Telegram emoji icon IDs.
- **Behavior**:
  - Accepts ``-e | --emoji | --icon <custom_emoji_id>`` CLI parameter.
  - Passes ``icon_custom_emoji_id`` string parameter in TDLib ``createForumTopic`` and ``editForumTopic`` requests.
  - Allows zero or numeric 64-bit integer Telegram custom emoji identifier strings.

[FUNC-012] Project Branding & Visual Logo Assets
------------------------------------------------

- **Requirement**: The project must maintain vector and raster logo assets styled after modern Crystal shard/flaw geometric design patterns.
- **Behavior**:
  - Vector SVG logo located at ``assets/logo.svg`` and raster PNG located at ``assets/logo.png``.
  - Embedded into ``README.rst`` and Sphinx documentation templates.
  - Features geometric faceted crystal shard visuals with vibrant gradient tones symbolizing TDLib high-performance C++23 message processing.
