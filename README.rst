==================================
grm — Group & Telegram Manager CLI
==================================

**grm** is a high-performance, native **C++23** command-line interface for managing Telegram accounts, groups, message history, and data extractions. Powered by Telegram's official C++ engine **TDLib** (``libtdjson``).

Features
--------

- **TDLib Engine**: Powered by Telegram's official C++ engine (``libtdjson``), providing 100% MTProto coverage.
- **Zero AccessHash Peer Bugs**: Automatically handles channels and supergroups (``-100...`` IDs) without invalid peer errors.
- **Interactive Authentication**: Supports phone number authentication, SMS code validation, and 2FA password challenges.
- **Chat Management**: Lists channels, groups, and direct messages with IDs and titles.
- **Message Inspection**: Reads message history from any group or channel.
- **Birthday Data Extraction**: Scans message history with regex pattern matching to extract birthday registrations.
- **Message Sending**: Sends text messages directly to any chat or group.

Directory Structure
-------------------

.. code-block:: text

   grm/
   ├── CMakeLists.txt              # CMake build configuration (C++23)
   ├── GNUmakefile                 # GNU Makefile build system
   ├── README.rst                  # Project documentation
   ├── include/
   │   └── grm/
   │       ├── app.hpp             # Main CLI application controller
   │       ├── config.hpp          # Configuration loader
   │       ├── json_utils.hpp      # C++23 RAII JsonValue wrapper for json-c
   │       └── td_client.hpp       # Asynchronous TDLib client wrapper
   └── src/
       ├── main.cpp                # CLI entry point
       ├── app.cpp                 # Application logic & command handlers
       ├── config.cpp              # Configuration management
       ├── json_utils.cpp          # JSON parsing implementation
       └── td_client.cpp           # TDLib event loop and promise routing

Prerequisites (Fedora Linux)
----------------------------

Install required build dependencies:

.. code-block:: bash

   sudo dnf install tdlib-devel json-c-devel cmake ninja-build gcc-c++ clang-tools-extra

Building and Installing
-----------------------

Using the ``GNUmakefile``:

.. code-block:: bash

   # Build release binary (output: build/grm)
   make release

   # Install to ~/bin/grm
   make install-user

   # Format source files
   make format

   # Clean build artifacts
   make clean

Usage Guide
-----------

Global Flags:

- ``-h, --help``: Show help message.
- ``-V, --version``: Display application version.
- ``-v, --verbose``: Enable verbose TDLib state output.
- ``-q, --quiet``: Suppress non-essential informational messages.
- ``-c, --config <path>``: Specify custom config file path.

1. Account Authentication
~~~~~~~~~~~~~~~~~~~~~~~~~

Authenticate interactively or pre-fill your phone number:

.. code-block:: bash

   # Interactive login
   grm login

   # Pre-fill phone number
   grm login --phone +523335765013

2. List Active Chats & Groups
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

List chats with peer IDs, types, and titles:

.. code-block:: bash

   grm chat ls

3. Read Message History
~~~~~~~~~~~~~~~~~~~~~~~

Fetch the last 20 messages from a group (e.g. ``-1001789902965``):

.. code-block:: bash

   grm msg ls -1001789902965 20

4. Extract Birthday Registrations
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Scan history for birthday declarations:

.. code-block:: bash

   grm extract bday -1001789902965

5. Send a Message
~~~~~~~~~~~~~~~~~

Send a text message to a group or user:

.. code-block:: bash

   grm msg send -1001789902965 "Hola a todos desde grm CLI!"

6. Upload a File / Document
~~~~~~~~~~~~~~~~~~~~~~~~~~~

Upload and send a file:

.. code-block:: bash

   grm file send -1001789902965 /path/to/document.pdf "Document caption"

