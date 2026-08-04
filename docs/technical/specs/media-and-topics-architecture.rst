========================================================================
Technical Sub-Spec: File Uploads & Supergroup Forum Topics Architecture
========================================================================

Architecture Overview
=====================

This technical sub-specification details the TDLib integration patterns for local file uploads and Supergroup Forum Topics (message threads).

1. File Upload Architecture ([TECH-004])
========================================

Local file uploads leverage TDLib's ``sendMessage`` API combined with the ``inputMessageDocument`` or ``inputMessagePhoto`` types and the ``inputFileLocal`` structure.

TDLib Payload Structure for Local File Upload
---------------------------------------------

.. code-block:: json

   {
     "@type": "sendMessage",
     "chat_id": -1001789902965,
     "message_thread_id": 0,
     "input_message_content": {
       "@type": "inputMessageDocument",
       "document": {
         "@type": "inputFileLocal",
         "path": "/absolute/path/to/file.pdf"
       },
       "caption": {
         "@type": "formattedText",
         "text": "Optional file caption"
       }
     }
   }

Asynchronous File Progress Handling
------------------------------------

1. The CLI validates file existence and accessibility via ``std::filesystem::exists`` and ``std::filesystem::file_size``.
2. A request is dispatched to TDLib. For large files, ``updateFile`` events monitor upload progress until the payload transfer completes.

2. Supergroup Forum Topics Architecture ([TECH-005])
====================================================

Supergroups configured as forums contain separate message threads identified by a non-zero ``message_thread_id`` (representing the topic's root message ID).

Topic Discovery via TDLib API
-----------------------------

Forum topics are listed using TDLib's ``getForumTopics`` function:

.. code-block:: json

   {
     "@type": "getForumTopics",
     "chat_id": -1001789902965,
     "query": "",
     "offset_date": 0,
     "offset_message_id": 0,
     "offset_message_thread_id": 0,
     "limit": 100
   }

Thread-Scoped Message & File Dispatch
--------------------------------------

When dispatching text messages or uploading files to a specific topic thread, the ``message_thread_id`` field is set to the target topic thread ID in the ``sendMessage`` request:

.. code-block:: json

   {
     "@type": "sendMessage",
     "chat_id": -1001789902965,
     "message_thread_id": 42,
     "input_message_content": {
       "@type": "inputMessageText",
       "text": {
         "@type": "formattedText",
         "text": "Message sent into Topic Thread 42"
       }
     }
   }
