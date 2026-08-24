=====================================================================
ADR-002: Static Ahead-of-Time (AOT) Shell Autocompletion Architecture
=====================================================================

Date: 2026-08-24
Status: Accepted

Context
=======
Modern command-line tools implement shell autocompletion (Bash, Zsh, Fish) using either of two main paradigms:

1. **Dynamic Runtime Callbacks (JIT Proxy)**: Registering a lightweight shell function that forks and executes the application binary (e.g. ``grm __complete``) on every ``<Tab>`` keystroke to dynamically evaluate arguments and return suggestions.
2. **Static Ahead-of-Time (AOT) Generation**: Generating complete, self-contained shell completion scripts containing all commands, subcommands, and option flags at compile/build/installation time via introspection of the application's command registry.

We evaluated both approaches to determine the optimal autocompletion architecture for **grm**.

Decision
========
We decided to adopt the **Static Ahead-of-Time (AOT) Generation** model driven by introspection of the native ``CommandRegistry``:

* The binary provides a generator command (``grm completion <bash|zsh|fish>``) that outputs full, self-contained completion scripts directly from ``CommandRegistry``.
* Build systems (``GNUmakefile``, RPM packaging, installation targets) generate and place static scripts into standard OS completion directories (e.g., ``/usr/share/bash-completion/completions/grm`` or ``~/.local/share/bash-completion/completions/grm``).

Rationale
=========

1. **Safety & Zero Side-Effects**: Spawning the main application binary on every interactive ``<Tab>`` keystroke introduces risk. If a command-line parser inadvertently touches TDLib SQLite databases, writes logs, or triggers socket connections, partial or malformed input could cause database lock contention or unintentional mutations. Pure static shell scripts run entirely within shell memory with zero disk/database I/O.
2. **Zero Shell Freezes & Determinism**: If a dynamically invoked binary hangs (due to database locks, corrupted configuration files, or background thread deadlocks), the user's interactive shell prompt freezes. Static scripts guarantee deterministic completion with zero risk of hanging the shell.
3. **Security & Context Isolation**: Dynamic completion passes unparsed, partially typed CLI tokens (which may include credentials or unescaped characters) to a newly spawned process on every keystroke, expanding the attack surface regarding ``$PATH`` hijacking or ``LD_PRELOAD`` injection.
4. **Instantaneous Execution (0ms Latency)**: Static scripts execute immediately in-process without process-spawning overhead, maintaining responsiveness over slow SSH sessions, high-load systems, or NFS-mounted user homes.
5. **Zero Maintenance Burden via Single Source of Truth**: Because ``grm completion <shell>`` generates the static scripts directly from ``CommandRegistry``, developers never hand-edit shell scripts. Adding a new subcommand or flag automatically updates generated completion artifacts during ``make install`` or package builds.

Consequences
============

* Autocompletion does not query live TDLib runtime data (e.g., real-time chat IDs or dynamic message IDs) during ``<Tab>`` completion.
* Packaging and installation scripts are responsible for writing ``grm completion bash`` to the appropriate completion directories during build/install phases.
* The committed completion script in ``completions/grm.bash`` is kept in sync with ``CommandRegistry`` via automated build and check targets.
