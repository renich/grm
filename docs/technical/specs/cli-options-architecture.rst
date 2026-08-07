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
-------------------------

Subcommands parse GNU options using clean helper loops with ``std::from_chars`` for exception-free numeric parsing:

- **Message Listing**: ``grm msg ls [-n|--limit <N>] <chat_id>``
  - Option ``-n, --limit``: limit (int32, default 20)
  - Operands: ``chat_id`` (int64)
- **Message Search**: ``grm msg search [-q|--query "<pattern>"] [-n|--limit <N>] <chat_id>``
  - Option ``-q, --query``: query regex string
  - Option ``-n, --limit``: limit (int32, default 100)
  - Operands: ``chat_id`` (int64)
- **Message Export**: ``grm msg export [-f|--format csv|json] [-o|--output <file>] [-n|--limit <N>] <chat_id>``
  - Option ``-f, --format``: format type (default ``json``)
  - Option ``-o, --output``: output file path
  - Option ``-n, --limit``: max records (default 1000)
  - Operands: ``chat_id`` (int64)
- **Message & File Dispatch**: ``grm msg send [-a|--attach <file>] [-m|--media] [-C|--caption "<text>"] [-t|--topic <id>] <chat_id> ["<message>"]``
  - Option ``-a, --attach``: file attachment path (repeatable)
  - Option ``-m, --media``: media mode flag
  - Option ``-C, --caption``: caption string
  - Option ``-t, --topic``: topic ID (int64)
  - Operands: ``chat_id`` (int64), optional ``message`` text

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
