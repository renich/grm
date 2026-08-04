====================================================
Technical Sub-Spec: Forum Topics Engine Architecture
====================================================

Architecture Overview
=====================

This technical sub-specification details the TDLib integration patterns for Supergroup Forum Topics (message threads) ([TECH-005]).

Topic Discovery via TDLib API
-----------------------------

Forum topics are retrieved using TDLib's ``getForumTopics`` function:

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
