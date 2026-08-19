.. image:: assets/logo.svg
   :alt: grm — Group & Telegram Manager CLI
   :align: center
   :width: 360px

==================================
grm — Group & Telegram Manager CLI
==================================

**grm** is a high-performance, native **C++23** command-line tool for managing Telegram accounts, supergroups, message history, forum topics, file attachments, and data extractions. Powered directly by Telegram's official C++ engine **TDLib** (``libtdjson``).

:Pipeline Status: `GitLab CI Pipelines <https://gitlab.com/renich/grm/-/pipelines>`_
:License: `GPL-3.0-or-later <https://gitlab.com/renich/grm/-/blob/master/LICENSE>`_
:Language: C++23 (ISO/IEC 14882:2023)
:Engine: Telegram TDLib 1.8.66 (``libtdjson``)
:Primary Repository: `GitLab (gitlab.com/renich/grm) <https://gitlab.com/renich/grm>`_
:GitHub Mirror: `GitHub (github.com/renich/grm) <https://github.com/renich/grm>`_

Why grm?
--------

* **Dual Human/AI UX Engine**: Optimized both for interactive terminal use (ANSI TTY tables, color schemes) and automated AI agent workflows (JSON envelope, NDJSON streams).
* **Native TDLib Engine**: Direct C++ bindings to ``libtdjson`` for zero-hallucination, full MTProto protocol fidelity.
* **Universal Cross-Domain Search Suite**: Fast multi-entity discovery across chats, supergroups, channels, users, messages, and files (``grm search``).
* **Chat Folder Management**: Organize and filter chats by custom categories and Telegram chat list folders (``grm folder``).
* **Supergroup & Forum Topic First**: Complete lifecycle management for Telegram Supergroups, Forum Topics, custom emoji icons, and thread messages.
* **File Upload & Download Engine**: Streamlined document, photo, video, and media extractions with MIME detection and progress tracking.
* **Shell Tab Auto-Completion**: Context-aware Bash tab completion covering commands, subcommands, flags, and options.
* **Telegram Rich Text & Emoji Customization**: Telegram Markdown V2 entity formatting, inline message replies (``--reply-to``), and custom Supergroup topic emoji icons.
* **FreeDesktop & XDG Compliant**: Strictly honors FreeDesktop standards for user binaries, man pages, shell completions, and session state.

FreeDesktop & XDG Configuration Standards
-----------------------------------------

**grm** strictly honors the **FreeDesktop XDG Base Directory Specification** and **FHS 3.0**:

=================================================== ====================================================================
File / Directory                                    Purpose
=================================================== ====================================================================
``~/.config/grm/config.json``                       Primary configuration file containing API credentials (``api_id``, ``api_hash``)
``~/.local/lib/grm/tdlib_db/``                      Persistent TDLib session database, encryption keys, and cache
``~/.local/bin/grm``                                User binary installation path (non-root)
``~/.local/share/man/man1/grm.1``                   User manual page installation path
``~/.local/share/bash-completion/completions/grm``  User Bash tab autocompletion script
=================================================== ====================================================================

Dependencies
------------

User / Build Dependencies
~~~~~~~~~~~~~~~~~~~~~~~~~

To build and run **grm**, end users need the following C++ libraries and build toolchain:

=================== ================== =========================================================
Dependency          Minimum Version    Purpose
=================== ================== =========================================================
``tdlib-devel``     ``>= 1.8.0``       Telegram C++ JSON interface library (``libtdjson``)
``json-c-devel``    ``>= 0.15``        C JSON parser and object serialization engine
``cmake``           ``>= 3.25``        Cross-platform build system generator
``ninja-build``     ``>= 1.10``        Fast build execution engine
``gcc-c++`` / ``clang`` ``C++23``     C++23 compiler suite (ISO/IEC 14882:2023)
``python3-docutils`` ``>= 0.18``      reStructuredText man page compiler (``rst2man``)
=================== ================== =========================================================

Installing User Dependencies (Fedora Linux / RHEL)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. code-block:: bash

   sudo dnf -y install \
       tdlib-devel \
       json-c-devel \
       cmake \
       ninja-build \
       gcc-c++ \
       python3-docutils \
       git

Developer & Quality Toolchain Dependencies
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Contributors running static analysis (``make lint``), code formatting (``make format``), or doc validation (``make doc-check``) also require:

===================== ================== =========================================================
Dependency            Minimum Version    Purpose
===================== ================== =========================================================
``clang-tools-extra`` ``>= 16.0``        Static analysis (``clang-tidy``) and formatter
``rstcheck``          ``>= 6.0``         reStructuredText syntax validator
===================== ================== =========================================================

Installing Developer Dependencies (Fedora Linux / RHEL)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. code-block:: bash

   sudo dnf -y install \
       clang-tools-extra \
       rstcheck

Quick Start
-----------

Building from Source
~~~~~~~~~~~~~~~~~~~~

.. code-block:: bash

   # Clone repository
   git clone https://gitlab.com/renich/grm.git
   cd grm

   # Build debug binary (default)
   make

   # Build optimized release binary
   make release

   # Run automated test suite
   make check

   # (Optional for developers) Run linters & static analysis
   make lint

Installing Locally (Non-Root User / No Sudo Required)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Per **FHS 3.0** and the **XDG Base Directory Specification**, install **grm** into your user profile without root privileges:

.. code-block:: bash

   # Installs binary to ~/.local/bin/grm, man page to ~/.local/share/man/man1/grm.1, and completion
   make install-user

   # Ensure ~/.local/bin is in your PATH (e.g. in ~/.bashrc or ~/.bash_profile):
   export PATH="$HOME/.local/bin:$PATH"

Setting Up Shell Auto-Completion
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

If installed via ``make install-user``, Bash autocompletion is placed in ``~/.local/share/bash-completion/completions/grm``.

To enable completion for your current shell session:

.. code-block:: bash

   source "$HOME/.local/share/bash-completion/completions/grm"

To enable completion permanently, ensure your ``~/.bashrc`` loads user completions or add the source line above.

Installing System-Wide (Requires Sudo)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

To install **grm** system-wide for all users:

.. code-block:: bash

   # Installs binary to /usr/local/bin/grm and man page to /usr/local/share/man/man1/grm.1
   sudo make install

   # Or specify a custom PREFIX:
   make release && sudo make install PREFIX=/usr

First-Time Authentication
~~~~~~~~~~~~~~~~~~~~~~~~~

Before executing chat or message commands, authenticate **grm** with your Telegram account:

.. code-block:: bash

   # Interactive authentication flow (prompts for phone, verification code, and 2FA password)
   grm login

   # Or pass your phone number directly:
   grm login --phone +523312345678

Usage Highlights
----------------

.. code-block:: bash

   # Authenticate Telegram account
   grm login

   # List active chats filtered by custom folder
   grm chat ls --folder 1

   # Search supergroups and channels globally across Telegram
   grm search supergroups "devops"

   # Search message history with sender and regex filters
   grm msg search -q "error" --sender "@admin" -1003750297693

   # Send inline message reply to a specific message ID
   grm msg send -r 42 -t 2 -1003750297693 "Acknowledged and resolved."

   # Create a supergroup forum topic with a custom emoji icon
   grm topic create -e 5368560552786271734 -1003750297693 "DevOps Operations"

   # Send a message with Telegram Markdown V2 formatting
   grm msg send -t 2 -1003750297693 "Deployment **successful** to production!"

   # Send document attachment with custom caption
   grm msg send -a /var/log/syslog -C "System logs" -t 2 -1003750297693

   # Export topic message history as JSON for AI LLM context
   grm msg export -f json -o history.json -t 2 -1003750297693

   # Bulk download all photos from a forum topic
   grm file download-all -o ~/Downloads -t 2 --type photo -1003750297693

   # Log out from Telegram and clear session credentials
   grm logout

Documentation & Specs
---------------------

For full manual pages, architecture specifications, and roadmaps:

* `User Guide & Options <docs/user/grm.rst>`_
* `Man Page (man grm) <docs/man/grm.1.rst>`_
* `Functional Specification <docs/functional/spec.rst>`_
* `Technical Architecture Specification <docs/technical/spec.rst>`_
* `Project Roadmap <docs/project/roadmap.rst>`_
* `Contributing Guidelines <CONTRIBUTING.rst>`_

License
-------

Distributed under the terms of the `GNU General Public License v3.0 or later (GPL-3.0-or-later) <LICENSE>`_.
