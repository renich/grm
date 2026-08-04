==============================================
Technical Architecture: CLI Options & Logging
==============================================

Overview
--------

This architecture document details the C++23 design for global option parsing, verbosity control, and structured terminal logging in **grm**.

Data Structures
---------------

CliOptions Struct
~~~~~~~~~~~~~~~~~

Defined in ``include/grm/config.hpp``:

.. code-block:: cpp

   enum class VerbosityLevel {
     Quiet = 0,
     Normal = 1,
     Verbose = 2,
     Debug = 3
   };

   struct CliOptions {
     std::string phone;
     std::string code;
     std::filesystem::path config_path;
     VerbosityLevel verbosity{VerbosityLevel::Normal};
     bool use_test_dc{false};
     bool help{false};
     bool version{false};
   };


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

Command Line Parser
-------------------

The CLI argument parser iterates over ``argv`` using non-throwing ``std::string_view`` checks. Global options are extracted into ``CliOptions`` before dispatching to specific command handlers.

TDLib Update Filtering
----------------------

Internal TDLib updates (e.g. ``authorizationStateWaitTdlibParameters``) are routed through ``grm::log::debug()`` and will only print when ``verbosity >= VerbosityLevel::Verbose``.
User-facing auth steps (e.g. ``Enter authentication code:``) are routed through ``grm::log::auth()``.
