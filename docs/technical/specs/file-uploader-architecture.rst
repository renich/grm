=====================================================
Technical Sub-Spec: File Uploader Engine Architecture
=====================================================

Architecture Overview
=====================

This technical sub-specification details the TDLib integration patterns for local file and media attachment uploads ([TECH-004]).

File Upload Payload Construction
--------------------------------

Local file uploads leverage TDLib's ``sendMessage`` API combined with the ``inputMessageDocument`` or ``inputMessagePhoto`` types and the ``inputFileLocal`` structure.

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

Asynchronous Upload Lifecycle
-----------------------------

1. **Validation**: The CLI validates local file existence, readability, and file size via ``std::filesystem::exists`` and ``std::filesystem::file_size``.
2. **Dispatch**: The request payload is formatted and sent asynchronously through ``grm::TdClient::send_request``.
3. **Progress Tracking**: ``updateFile`` events monitor upload byte progress until TDLib emits completion status.
