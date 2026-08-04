==================================
grm — Group & Telegram Manager CLI
==================================

**grm** is a high-performance, native **C++23** command-line interface for managing Telegram accounts, groups, message history, forum topics, file uploads, and data extractions. Powered by Telegram's official engine **TDLib** (``libtdjson``).

Key Features
------------

- **TDLib Engine**: Direct C++ integration with ``libtdjson`` for complete MTProto coverage.
- **Zero AccessHash Peer Bugs**: Automatically resolves supergroup and channel IDs (``-100...``) without invalid peer errors.
- **Interactive & Pre-filled Auth**: Supports interactive login, pre-filled phone/code options, SMS resend, and 2FA password challenges.
- **Chat & Forum Topic Management**: List active chats and inspect supergroup forum topics.
- **Message Operations**: Read, export (CSV/JSON), and regex search message histories.
- **File & Document Uploads**: Upload files and documents with captions and topic target flags.
- **Data Extraction**: Scan chat history with regex patterns to extract registered user birthdays.
- **Strict Quality Suite**: 100% C++23 standards compliance with Clang static analysis and Clang-Tidy linters.

Directory Layout
----------------

.. code-block:: text

   grm/
   ├── CMakeLists.txt              # C++23 CMake build configuration
   ├── GNUmakefile                 # Build, test, lint, and install targets
   ├── README.rst                  # Project documentation
   ├── docs/                       # Functional, technical specs, and ADRs
   ├── include/
   │   └── grm/
   │       ├── app.hpp             # Main CLI application controller
   │       ├── config.hpp          # Configuration & CLI option loader
   │       ├── exporter.hpp        # CSV and JSON chat history exporter
   │       ├── json_utils.hpp      # C++23 RAII JsonValue wrapper for json-c
   │       ├── logger.hpp          # Structured level logger ([INFO], [AUTH], etc.)
   │       ├── td_client.hpp       # Asynchronous TDLib client wrapper
   │       └── uploader.hpp        # Document and file payload builder
   ├── src/                        # Implementation sources
   └── tests/                      # Unit and integration tests

Prerequisites (Fedora Linux)
----------------------------

Install required build dependencies:

.. code-block:: bash

   sudo dnf install tdlib-devel json-c-devel cmake ninja-build gcc-c++ clang-tools-extra

Building & Testing
------------------

Using the ``GNUmakefile``:

.. code-block:: bash

   # Development build (unoptimized, -O0 -g3 debug symbols)
   make

   # Optimized production build (-O3 -DNDEBUG, stripped)
   make release

   # Run test suite
   make check

   # Run static analyzer and linters
   make lint
   make analyze

   # Install binary to ~/bin/grm
   make install-user

Command Reference
-----------------

Global Options (GNU Long & POSIX Short Pairs)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

- ``-h, --help``: Display help screen and exit.
- ``-V, --version``: Display version information and exit.
- ``-v, --verbose``: Enable verbose TDLib state output.
- ``-d, --debug``: Enable raw JSON payload tracing.
- ``-q, --quiet``: Suppress non-essential informational output.
- ``-c, --config <path>``: Custom configuration file path.
- ``-T, --test-dc``: Connect to Telegram Test Data Center environment.


Command Options
~~~~~~~~~~~~~~~

- Login: ``-p, --phone <number>``, ``-k, --code <code>``
- Send File: ``-C, --caption "<text>"``, ``-t, --topic <id>``

Usage Examples
~~~~~~~~~~~~~~

1. Account Authentication:
   
   .. code-block:: bash

      # Interactive login
      grm login

      # Pre-fill phone number (GNU short option -p / long option --phone)
      grm login -p +523335765013

2. Chat & Forum Topic Navigation:

   .. code-block:: bash

      # List active chats and supergroups
      grm chat ls

      # List forum topics in a supergroup
      grm topic ls -1001789902965

3. Message Inspection & Search:

   .. code-block:: bash

      # Read recent messages from a chat
      grm msg ls -1001789902965 20

      # Search chat history with regex filter
      grm msg search -1001789902965 "reunión"

      # Export chat history to CSV or JSON
      grm msg export -1001789902965 csv history.csv

4. File & Document Uploads:

   .. code-block:: bash

      # Send a text message
      grm send -1001789902965 "Hola desde grm CLI!"

      # Upload a document with caption (-C / --caption)
      grm send file -1001789902965 /path/to/report.pdf -C "Monthly Report"

      # Upload a document directly to a forum topic (-t / --topic)
      grm send file -1001789902965 /path/to/doc.pdf -C "Topic Doc" -t 42

5. Birthday Data Extraction:

   .. code-block:: bash

      # Scan history for birthday announcements
      grm extract bday -1001789902965

