==========================================================
Technical Architecture: Formatter & Output Distribution
==========================================================

Overview
--------

This specification describes the C++23 design for the **grm::formatter** domain module. The module encapsulates output formatting, ANSI color palette management, type humanization dictionaries, and structured table generation across Human, Markdown, JSON, and Plain output modes.

Architecture & Domain Boundaries
--------------------------------

The formatting architecture is organized into clean domain headers and source files:

- ``include/grm/formatter.hpp``: Public API declarations for format modes, ANSI color wrappers, human type transformers, and table renderers.
- ``src/formatter.cpp``: Implementation of ANSI styling, TTY detection (``isatty``), string alignment, and format rendering.

Data Types & Enums
------------------

Output Format & Color Mode Enums
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. code-block:: cpp

   namespace grm::fmt {

   enum class OutputFormat {
     Auto,      // Human-readable grid for TTY, plain for non-TTY
     Human,     // Rich ANSI colored grid with human labels
     Markdown,  // GitHub Flavored Markdown tables
     Json,      // Structured JSON serialization
     Plain      // Grid alignment without ANSI colors
   };

   enum class ColorMode {
     Auto,    // Enable colors if stdout is TTY and NO_COLOR is unset
     Always,  // Force ANSI color codes
     Never    // Suppress all ANSI color codes
   };

   } // namespace grm::fmt

ANSI Color Palette Manager
~~~~~~~~~~~~~~~~~~~~~~~~~~

The formatter provides lightweight, zero-allocation ANSI styling helpers:

- **Reset**: ``\033[0m``
- **Bold**: ``\033[1m``
- **Dim**: ``\033[2m``
- **Cyan** (Chat IDs / Accents): ``\033[36m``
- **Green** (Titles / Success): ``\033[32m``
- **Yellow** (Topics / Warnings): ``\033[33m``
- **Blue** (Info / Badges): ``\033[34m``
- **Red** (Errors): ``\033[31m``

Type Humanization Mappings
~~~~~~~~~~~~~~~~~~~~~~~~~~

Raw TDLib JSON type strings are mapped using non-allocating ``std::string_view`` lookup functions:

.. code-block:: cpp

   std::string_view humanize_chat_type(std::string_view tdlib_type);
   std::string_view humanize_auth_code_type(std::string_view tdlib_type);

Table & List Rendering Interface
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The ``Formatter`` class presents clean static and instance methods for rendering data structures:

.. code-block:: cpp

   struct ChatItem {
     int64_t id;
     std::string type;
     std::string title;
   };

   struct TopicItem {
     int64_t id;
     std::string name;
     int64_t message_count;
   };

   class Formatter {
   public:
     static void print_chats(const std::vector<ChatItem> &chats, OutputFormat format, ColorMode color_mode);
     static void print_topics(const std::vector<TopicItem> &topics, OutputFormat format, ColorMode color_mode);
   };

TTY Auto-Detection Logic
------------------------

Colors are automatically disabled if:
1. ``color_mode == ColorMode::Never``
2. ``color_mode == ColorMode::Auto`` AND ``!isatty(STDOUT_FILENO)``
3. The ``NO_COLOR`` environment variable is set and non-empty.
