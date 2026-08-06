====================================================
Technical Sub-Spec: TDLib Client Architecture & RAII
====================================================

Module Specification
====================

* **[TECH-001]**: C++23 TDLib Client Wrapper (``grm::TdClient``)
* **[TECH-002]**: C++23 RAII JSON Wrapper (``grm::JsonValue``)
* **[TECH-003]**: Application Controller (``grm::App``)

[TECH-001] TDLib Client Wrapper (grm::TdClient)
-----------------------------------------------

* **Fulfills Requirements**: ``[FUNC-001]``, ``[FUNC-002]``
* **Header**: ``include/grm/td_client.hpp``
* **Implementation**: ``src/td_client.cpp``

The ``TdClient`` class wraps TDLib's C JSON API (``td_create_client_id``, ``td_send``, ``td_receive``):

* **Lifecycle**: Manages background worker thread ``receiver_thread_`` executing ``receiver_loop()``.
* **Request Routing**: Generates atomic request identifiers (``req_1``, ``req_2``, ...) attached to the ``@extra`` field of sent JSON objects. Incoming responses matching ``@extra`` set corresponding ``std::promise<JsonValue>`` objects.
* **Synchronous Wait**: ``send_request(type, payload, timeout)`` waits on the resulting ``std::future<JsonValue>`` and returns ``std::expected<JsonValue, std::string>``.

[TECH-002] RAII JSON Wrapper (grm::JsonValue)
---------------------------------------------

* **Fulfills Requirements**: ``[FUNC-003]``
* **Header**: ``include/grm/json_utils.hpp``
* **Implementation**: ``src/json_utils.cpp``

Wraps C library ``json-c`` pointers with modern C++ memory management:

* **RAII Ownership**: Calls ``json_object_get`` on copy/increment and ``json_object_put`` on destruction, preventing memory leaks.
* **Type-Safe Getters**: Provides ``get_string()``, ``get_int()``, ``get_bool()``, ``get_object()``, and ``get_array()`` returning ``std::optional``.

[TECH-003] Application Controller (grm::App)
--------------------------------------------

* **Fulfills Requirements**: ``[FUNC-005]``
* **Header**: ``include/grm/app.hpp``
* **Implementation**: ``src/app.cpp``

Executes CLI subcommand routing:

* ``cmd_login()``: Drives interactive phone and password authentication state transitions.
* ``cmd_chat_ls()``: Fetches chat IDs via ``getChats`` and inspects details via ``getChat``.
* ``cmd_msg_ls()``: Retrieves history via ``getChatHistory``.
* ``cmd_send()``: Dispatches formatted text via ``sendMessage``.

