==============================================
Technical Architecture: Shell Autocompletion
==============================================

Overview
--------

This technical architecture document describes the design of subcommand-level help dispatch in C++23 and the completion generator logic in Bash.

C++23 Subcommand Help Dispatch
------------------------------

In ``src/app.cpp``, ``App::run()`` checks for ``-h`` or ``--help`` in subcommand argument vectors before executing command logic:

.. code-block:: cpp

   bool is_help_requested(const std::vector<std::string> &args) {
     for (const auto &arg : args) {
       if (arg == "-h" || arg == "--help") {
         return true;
       }
     }
     return false;
   }

When ``is_help_requested()`` is true, execution routes to dedicated help functions:
- ``print_login_help()``
- ``print_chat_help()``
- ``print_msg_help()``
- ``print_send_help()``
- ``print_topic_help()``
- ``print_extract_help()``

Bash Autocompletion Engine
--------------------------

File: ``completions/grm.bash``

The script hooks into Bash completion via ``complete -F _grm_completions grm``:

1. **Top-Level Commands**:
   Completes ``login``, ``chat``, ``msg``, ``send``, ``topic``, ``extract``, and global flags (``-h``, ``--help``, ``-V``, ``--version``, ``-v``, ``--verbose``, ``-d``, ``--debug``, ``-q``, ``--quiet``, ``-c``, ``--config``).

2. **Subcommands & Flags**:
   - ``grm login`` $\rightarrow$ ``-p``, ``--phone``, ``-k``, ``--code``
   - ``grm chat`` $\rightarrow$ ``ls``
   - ``grm msg`` $\rightarrow$ ``ls``, ``export``, ``search``
   - ``grm msg export <chat_id>`` $\rightarrow$ ``csv``, ``json``
   - ``grm send`` $\rightarrow$ ``file``
   - ``grm send file`` $\rightarrow$ ``-C``, ``--caption``, ``-t``, ``--topic``
   - ``grm topic`` $\rightarrow$ ``ls``
   - ``grm extract`` $\rightarrow$ ``bday``
