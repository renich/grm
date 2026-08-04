==============================================
Functional Specification: CLI Options & Output
==============================================

Overview
--------

This specification defines the user-facing CLI flag interface and output formatting guidelines for **grm**. It establishes global options, non-interactive login parameters, verbosity levels, and clean, professional terminal output formatting.

Output Design Principles
------------------------

1. **No Emoji Clutter**: Terminal output must be clean, professional, and free of emojis.
2. **Structured Level Prefixes**: Output messages use clear, bracketed level indicators:
   - ``[INFO]``: General status and informational messages.
   - ``[WARN]``: Warning alerts and retries.
   - ``[ERROR]``: Fatal errors and failed operations.
   - ``[AUTH]``: Authentication progress and state transitions.
3. **Controlled Verbosity**: Internal TDLib protocol updates (e.g. ``authorizationStateWaitTdlibParameters``) are suppressed by default in normal mode and only displayed when ``--verbose`` or ``--debug`` is set.

Global CLI Flags
----------------

The CLI accepts global flags preceding or following command targets:

.. code-block:: text

   grm [GLOBAL_OPTIONS] <COMMAND> [COMMAND_OPTIONS]

Global Options:
~~~~~~~~~~~~~~~

- ``-h, --help``: Display structured usage help screen and exit.
- ``-V, --version``: Display version and build information and exit.
- ``-v, --verbose``: Enable detailed progress logging and TDLib state transitions.
- ``-d, --debug``: Enable raw TDLib JSON request/response payload tracing.
- ``-q, --quiet``: Suppress non-essential output (output only command results or errors).
- ``-c, --config <path>``: Specify custom configuration file path (default: ``~/.tgcli/config.json``).

Login Command Flags
-------------------

The ``login`` command accepts pre-filled authentication flags:

.. code-block:: text

   grm login [OPTIONS]

Options:
~~~~~~~~

- ``-p, --phone <number>``: Pre-fill Telegram phone number in E.164 format (e.g. ``+523335765013``).
- ``-c, --code <code>``: Supply authentication code directly for non-interactive scripts.

Examples
--------

Pre-filled Login:
~~~~~~~~~~~~~~~~~

.. code-block:: bash

   grm login --phone +523335765013

Verbose Diagnostic Execution:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. code-block:: bash

   grm --verbose chat ls
