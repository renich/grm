================
Coding Standards
================

Requirement Mapping
-------------------
Fulfills `[FUNC-001]`, `[FUNC-002]`, `[FUNC-003]`.

Core Directives
---------------

#. **Modern C++23 Standards**:
   Strict adherence to ISO C++23. Enforce `-Wall -Wextra -Wpedantic -Wconversion -Wshadow -Wnon-virtual-dtor -Wold-style-cast -Wcast-align -Wunused -Woverloaded-virtual -Wnull-dereference -Wdouble-promotion -Wformat=2 -Werror`.

#. **Memory Safety & Sanitizers**:
   Zero memory leaks and zero undefined behavior verified under AddressSanitizer (ASan), UndefinedBehaviorSanitizer (UBSan), and ThreadSanitizer (TSan).

#. **Error Handling & Ownership**:
   Enforce `std::expected` for explicit error propagation and return types. Use RAII with smart pointers (`std::unique_ptr`, `std::shared_ptr`) for resource management.

#. **Code Formatting**:
   Clang-Format compliant with 2-space indentation and max 132-character line limits.

#. **Test-Driven Verification**:
   All new features and bug fixes require comprehensive unit test suites in `tests/`.
