==============================================
Technical Architecture: CLI Options & Logging
==============================================

Overview
--------

This architecture document details the C++23 design for global option parsing, subcommand GNU flag parsing, verbosity control, and structured terminal logging in **grm**.

Data Structures
---------------

CliOptions Struct
~~~~~~~~~~~~~~~~~

Defined in ``include/grm/config.hpp``:

.. code-block:: cpp

   namespace grm {

   struct CliOptions {
     std::string phone;
     std::string code;
     std::filesystem::path custom_config_path;
     log::VerbosityLevel verbosity{log::VerbosityLevel::Normal};
     fmt::OutputFormat format{fmt::OutputFormat::Auto};
     fmt::ColorMode color_mode{fmt::ColorMode::Auto};
     bool use_test_dc{false};
     bool help{false};
     bool version{false};
   };

   } // namespace grm

Subcommand Option Parsers
------------------------

Subcommands parse GNU options using clean helper loops with ``std::from_chars`` for exception-free numeric parsing:

- **Message Listing**: ``grm msg ls <chat_id> [-n|--limit <N>]``
  - Positional 0: ``chat_id`` (int64)
  - Option ``-n, --limit``: limit (int32, default 20)
- **Message Search**: ``grm msg search <chat_id> [-q|--query "<pattern>"] [-n|--limit <N>]``
  - Positional 0: ``chat_id`` (int64)
  - Option ``-q, --query`` or Positional 1: query regex string
  - Option ``-n, --limit``: limit (int32, default 100)
- **Message Export**: ``grm msg export <chat_id> [-f|--format csv|json] [-o|--output <file>] [-n|--limit <N>]``
  - Positional 0: ``chat_id`` (int64)
  - Option ``-f, --format``: format type (default ``json``)
  - Option ``-o, --output``: output file path
  - Option ``-n, --limit``: max records (default 1000)
- **File Upload**: ``grm send file <chat_id> <file> [-C|--caption "<text>"] [-t|--topic <id>]``
  - Positional 0: ``chat_id`` (int64)
  - Positional 1: ``file_path`` (path)
  - Option ``-C, --caption``: caption string
  - Option ``-t, --topic``: topic ID (int64)

Logging Utility API
-------------------

Defined in ``include/grm/logger.hpp``:

.. code-block:: cpp

   namespace grm::log {

   void info(std::string_view msg);
   void warn(std::string_view msg);
   void error(std::string_view msg);
   void auth(std::string_view msg);
   void debug(std::string_view msg);

   void set_verbosity(VerbosityLevel level);
   VerbosityLevel get_verbosity();

   } // namespace grm::log
