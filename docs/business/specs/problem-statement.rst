=================
Problem Statement
=================

Context
-------
Managing Telegram chats, channels, forum supergroups, media files, stories, and emoji statuses through standard desktop and mobile GUI applications introduces operational friction for systems engineers, DevOps leads, and automation agents. Desktop GUIs lack scriptable automation, headless operation, structured output formats (JSON/Markdown), and reproducible CLI workflows.

Solution
--------
``grm`` provides a native, type-safe C++23 command-line interface powered by Telegram's official TDLib engine. It enables complete CRUD automation across all Telegram domains with structured JSON/Markdown outputs, headless authentication, zero peer access hash bugs, and seamless integration with Linux scripting and containerized environments.
