=======================================================
Phase 7: Rich Text, Topic Emojis & Branding Roadmap
=======================================================

**Status**: Completed & Deployed

Overview
========

Phase 7 enhances the user experience and visual identity of **grm** by integrating Telegram Rich Text Markdown V2 formatting, Custom Supergroup Topic Emoji icons, and a distinctive Crystal Shard geometric visual logo brand.

Key Capabilities
================

1. **Rich Text Formatting Engine**:
   - Integration of TDLib ``parseTextEntities`` method.
   - Support for bold, italic, inline code, links, and code blocks.

2. **Supergroup Topic Emoji Customization**:
   - Support for ``-e | --emoji | --icon <custom_emoji_id>`` flags in ``grm topic create`` and ``grm topic edit``.
   - TDLib payload mapping for ``icon_custom_emoji_id``.

3. **Brand Visual Identity & Logo**:
   - Creation of high-resolution vector SVG (``assets/logo.svg``) and raster PNG (``assets/logo.png``) logo assets.
   - Styled after Crystal Shard geometric facet aesthetic.
   - README and Sphinx documentation integration.

Module Tasks
============

- [x] Implement ``App::parse_formatted_text`` with TDLib ``parseTextEntities``.
- [x] Integrate entity parsing into ``cmd_msg_send``.
- [x] Add ``-e / --emoji`` flag parsing in topic creation and editing.
- [x] Create vector SVG and PNG logo assets in Crystal Flaw Shard style.
- [x] Update ``README.rst`` and Sphinx specs with branding assets.
- [x] Validate all CTest unit tests (11/11 passing).
