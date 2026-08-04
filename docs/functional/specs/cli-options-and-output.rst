==============================================
Functional Specification: CLI Options & Output
==============================================

Overview
--------

This specification defines the user-facing GNU/POSIX CLI option interface and output formatting guidelines for **grm**. It establishes global options, non-interactive login parameters, verbosity levels, and clean terminal output formatting adhering to GNU Coding Standards.

GNU Option Design Principles
---------------------------

1. **Strict GNU Option Compatibility**:
   - Short options use a single dash (e.g. ``-n 50``).
   - Long options use double dashes (e.g. ``--limit 50`` or ``--limit=50``).
   - Positional operands are restricted strictly to primary target identifiers (e.g., ``<chat_id>`` or ``<file>``). Optional flags, limits, formats, and outputs are configured via explicit short/long options.
2. **No Emoji Clutter**: Terminal output is clean, professional, and free of emojis.
3. **Structured Level Prefixes**: Output messages use clear, bracketed level indicators:
   - ``[INFO]``: General status and informational messages.
   - ``[WARN]``: Warning alerts and retries.
   - ``[ERROR]``: Fatal errors and failed operations.
   - ``[AUTH]``: Authentication progress and state transitions.
4. **Controlled Verbosity**: Internal TDLib protocol updates (e.g. ``authorizationStateWaitTdlibParameters``) are suppressed by default in normal mode and only displayed when ``-v, --verbose`` or ``-d, --debug`` is set.

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
- ``-F, --format <fmt>``: Output format: ``human``, ``markdown``, ``json``, ``plain`` (default: ``auto``).
- ``--color <mode>``: Color mode: ``auto``, ``always``, ``never`` (or ``--no-color``).
- ``-T, --test-dc``: Connect to Telegram Test Data Center environment (isolated test DB and servers).

Subcommand GNU Options Matrix
-----------------------------

Login Command:
~~~~~~~~~~~~~~

- ``grm login [-p|--phone <number>] [-k|--code <code>]``
  - ``-p, --phone <number>``: Pre-fill Telegram phone number in E.164 format.
  - ``-k, --code <code>``: Pre-fill authentication code for non-interactive scripts.

Message Subcommands:
~~~~~~~~~~~~~~~~~~~

- ``grm msg ls [-n|--limit <N>] <chat_id>``
  - ``-n, --limit <N>``: Limit maximum messages to fetch (default: 20).
- ``grm msg search [-q|--query "<pattern>"] [-n|--limit <N>] <chat_id>``
  - ``-q, --query "<pattern>"``: Filter messages using regex search pattern.
  - ``-n, --limit <N>``: Scan limit (default: 100).
- ``grm msg export [-f|--format csv|json] [-o|--output <file>] [-n|--limit <N>] <chat_id>``
  - ``-f, --format <fmt>``: Export format (``json`` or ``csv``, default: ``json``).
  - ``-o, --output <file>``: Destination export file path.
  - ``-n, --limit <N>``: Maximum records to export (default: 1000).

Extract Subcommand:
~~~~~~~~~~~~~~~~~~

- ``grm extract bday [-n|--limit <N>] <chat_id>``
  - ``-n, --limit <N>``: Maximum messages to scan (default: 100).

Send Subcommands:
~~~~~~~~~~~~~~~~~

- ``grm send [-t|--topic <id>] <chat_id> "<message>"``
  - ``-t, --topic <id>``: Direct text message to a specific forum topic thread ID.
- ``grm send file [-C|--caption "<text>"] [-t|--topic <id>] <chat_id> <file>``
  - ``-C, --caption "<text>"``: Attach text caption to uploaded document.
  - ``-t, --topic <id>``: Direct document upload to a specific forum topic thread ID.

Examples
--------

GNU Formatted History Listing:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. code-block:: bash

   grm msg ls -n 50 -1002289735000

GNU Formatted Export with Explicit Output Path:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. code-block:: bash

   grm msg export -f csv -o /tmp/evalinux_history.csv -n 500 -1002289735000

Upload Document with Caption to Forum Topic:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. code-block:: bash

   grm send file -C "Monthly Report" -t 42 -1001789902965 /path/to/report.pdf

