==============================================
Functional Specification: CLI Options & Output
==============================================

Overview
--------

This specification defines the user-facing GNU/POSIX CLI option interface and output formatting guidelines for **grm**. It establishes global options, non-interactive login parameters, verbosity levels, and clean terminal output formatting adhering to GNU Coding Standards.

Output Design Principles
------------------------

1. **No Emoji Clutter**: Terminal output is clean, professional, and free of emojis.
2. **Structured Level Prefixes**: Output messages use clear, bracketed level indicators:
   - ``[INFO]``: General status and informational messages.
   - ``[WARN]``: Warning alerts and retries.
   - ``[ERROR]``: Fatal errors and failed operations.
   - ``[AUTH]``: Authentication progress and state transitions.
3. **Controlled Verbosity**: Internal TDLib protocol updates (e.g. ``authorizationStateWaitTdlibParameters``) are suppressed by default in normal mode and only displayed when ``-v, --verbose`` or ``-d, --debug`` is set.

Global Options (GNU Long & POSIX Short Pairs)
---------------------------------------------

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
- ``-c, --config <path>``: Specify custom configuration file path (default: ``~/.config/grm/config.json``).
- ``-T, --test-dc``: Connect to Telegram Test Data Center environment (isolated test DB and servers).


Command Options
---------------

Login Command:
~~~~~~~~~~~~~~

- ``-p, --phone <number>``: Pre-fill Telegram phone number in E.164 format (e.g. ``+523335765013``).
- ``-k, --code <code>``: Pre-fill authentication code for non-interactive scripts.

Send File Command:
~~~~~~~~~~~~~~~~~~

- ``-C, --caption "<text>"``: Attach text caption to uploaded document.
- ``-t, --topic <id>``: Direct document upload to a specific forum topic thread ID.

Examples
--------

Pre-filled Login:
~~~~~~~~~~~~~~~~~

.. code-block:: bash

   grm login -p +523335765013

Upload Document to Forum Topic:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. code-block:: bash

   grm send file -1001789902965 /path/to/report.pdf -C "Monthly Report" -t 42

Verbose Diagnostic Execution:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. code-block:: bash

   grm -v chat ls

Test Data Center Authentication:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. code-block:: bash

   grm -T login -p +9996612345 -k 22222

