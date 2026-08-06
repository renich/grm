=========================================================
ADR-001: TDLib & C++23 Native Architecture Choice for grm
=========================================================

Date: 2026-08-03
Status: Accepted

Context
=======
The team required a fast, highly reliable command-line tool for Telegram personal account management, chat discovery, message history inspection, and group data management. Third-party MTProto wrappers (such as ``kaosb/tgcli``) suffered from unresolved ``AccessHash`` peer resolution defects when querying supergroups (IDs starting with ``-100...``), resulting in ``CHANNEL_INVALID`` API errors.


Decision
========
We decided to build **grm** as a native **C++23** CLI application linking directly against Telegram's official C++ engine **TDLib** (``libtdjson.so``).

Rationale
=========

1. **Official Engine**: TDLib is authored and maintained directly by Telegram core developers, guaranteeing 100% MTProto feature coverage and day-zero compatibility with server updates.
2. **Autonomous Peer Resolution**: TDLib's internal state machine handles peer access hashes, connection pooling, and local encrypted SQLite state automatically, eliminating peer resolution bugs.
3. **Modern C++ Standards**: Adopting C++23 (via GCC 16 / CMake / Ninja) provides strong type safety (``std::expected``, RAII smart pointers, monadic operations) and high runtime performance.

Consequences
============

* The project links against ``libtdjson.so`` and ``libjson-c.so``.
* Building from source requires ``tdlib-devel``, ``json-c-devel``, ``cmake``, and ``ninja-build`` on Fedora Linux.
