============================================================
Functional Specification: Human & LLM Friendly Output Engine
============================================================

Overview
--------

This specification defines the output rendering engine for **grm**. The CLI provides visually appealing, human-friendly terminal formatting with a splash of color by default, while supporting machine and LLM-optimized formats (Markdown tables, JSON, plain text) when piped or explicitly requested.

Key Features & Requirements
---------------------------

1. **Human-Friendly Defaults (TTY Mode)**:

   - **Splash of Color**: Styled headers, colored level badges (``[INFO]``, ``[WARN]``, ``[ERROR]``, ``[AUTH]``), and distinct accent colors for Chat IDs (Cyan) and Chat Titles (Green).
   - **Humanized Naming Mappings**: Raw TDLib internal type strings are converted into intuitive human names:

     - ``chatTypeSupergroup`` → ``Supergroup``
     - ``chatTypeBasicGroup`` → ``Basic Group``
     - ``chatTypePrivate`` → ``Private Chat``
     - ``chatTypeSecret`` → ``Secret Chat``
     - ``authenticationCodeTypeTelegramMessage`` → ``In-App Message``
     - ``authenticationCodeTypeSms`` → ``SMS``
     - ``authenticationCodeTypeCall`` → ``Phone Call``

2. **LLM & Pipeline Friendly Formats**:

   - **Markdown Mode (``--format markdown``)**: Generates GitHub-flavored Markdown tables and headers optimized for LLM prompting and document insertion.
   - **JSON Mode (``--format json``)**: Outputs clean, uncolored JSON arrays or objects for automated LLM pipelines and Unix tools (e.g. ``jq``).
   - **Plain Text Mode (``--format plain``)**: Suppresses ANSI color escape codes while retaining readable spacing.

3. **Multi-Line Message Alignment & Markdown Escaping**:

   - **Interactive Terminal Indentation**: Embedded newlines (``\n``) in multi-line message texts are split and continuation lines are indented to match the width of the ``[MsgID <id>]:`` prefix.
   - **Markdown Table Escaping**: Embedded newlines inside Markdown table cells are replaced with ``<br>`` tags to preserve Markdown table alignment and parser compatibility.

4. **TTY Auto-Detection & Color Control**:

   - **Automatic Color Suppression**: ANSI color codes are automatically disabled when stdout is redirected to a pipe or file (via ``isatty(STDOUT_FILENO)``).
   - **Environment & Flags**: Honor the standard ``NO_COLOR`` environment variable and explicit CLI flags (``--color=always|never|auto``).

Output Format Matrix
--------------------

+-----------------------+-----------------------------+---------------------------------+------------------------+
| Mode / Flag           | Visual Styling              | Type Labels                     | Target Audience        |
+=======================+=============================+=================================+========================+
| Default (Interactive) | ANSI Colors & Aligned Grid  | Humanized (e.g. ``Supergroup``) | Human Terminal User    |
+-----------------------+-----------------------------+---------------------------------+------------------------+
| ``--format markdown`` | GitHub Markdown Tables      | Humanized                       | LLM Context / Docs     |
+-----------------------+-----------------------------+---------------------------------+------------------------+
| ``--format json``     | Structured JSON Objects     | Standard JSON Keys              | LLM Pipelines & Scripts|
+-----------------------+-----------------------------+---------------------------------+------------------------+
| ``--no-color`` / Pipe | Uncolored Aligned Grid      | Humanized                       | Shell Pipes & Logs     |
+-----------------------+-----------------------------+---------------------------------+------------------------+

Examples
--------

Human Interactive Output (Default):
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. code-block:: text

   CHAT ID              TYPE            TITLE
   ------------------------------------------------------------
   -1003750297693       Supergroup      TestGroup
   777000               Private Chat    Telegram

Markdown Output for LLM Prompting (``grm chat ls --format markdown``):
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. code-block:: markdown

   | Chat ID | Type | Title |
   | :--- | :--- | :--- |
   | -1003750297693 | Supergroup | TestGroup |
   | 777000 | Private Chat | Telegram |
