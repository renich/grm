==========================================================
Functional Specification: Subcommand Help & Autocompletion
==========================================================

Overview
--------

This specification details subcommand-level help screen handling (e.g. ``grm login --help``) and the multi-shell autocompletion architecture for **grm**.

Subcommand Help Screens
-----------------------

When ``-h`` or ``--help`` is passed alongside any subcommand, **grm** displays a focused help screen detailing that subcommand's usage, positional parameters, and accepted GNU/POSIX flags:

Subcommand Target Help Examples:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

- ``grm login -h, --help``:
  Displays phone/code pre-fill parameters and authentication instructions.

- ``grm chat -h, --help``:
  Displays chat and supergroup listing options.

- ``grm msg -h, --help``:
  Displays message listing (``ls``), searching (``search``), and exporting (``export``) parameters.

- ``grm send -h, --help``:
  Displays message and document upload options (``-C, --caption``, ``-t, --topic``).

- ``grm topic -h, --help``:
  Displays forum topic thread listing parameters.


Shell Autocompletion Architecture
---------------------------------

Autocompletion scripts reside in a dedicated top-level directory: ``completions/``.

Supported Shell Extensions:
~~~~~~~~~~~~~~~~~~~~~~~~~~~

- ``completions/grm.bash``: Native Bash completion script (primary).
- ``completions/_grm``: Zsh completion script interface (extensibility placeholder).
- ``completions/grm.fish``: Fish completion script interface (extensibility placeholder).

Installation Targets
--------------------

The build system provides dedicated installation targets:

- ``make install-completions``: Installs completion scripts to system directory ``/usr/share/bash-completion/completions/grm``.
- ``make install-user-completions``: Installs completion scripts to user directory ``~/.local/share/bash-completion/completions/grm``.
