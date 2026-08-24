==================================================
Phase 4 Roadmap: Supergroup Forum Topics (Threads)
==================================================

Phase Overview
==============

Phase 4 introduces Supergroup Forum Topics discovery and topic thread-scoped message/file dispatching to **grm**.

Milestones & Atomic Tasks
=========================

Milestone 4.1: Forum Topics Discovery & Thread-Scoped Messaging
---------------------------------------------------------------

* **Task 4.1.1**: Implement ``grm topic ls <supergroup_id>`` forum topics discovery command via TDLib ``getForumTopics``.

  * *Delivers*: ``src/cmd_topic.cpp``
  * *Fulfills*: ``[FUNC-008]``, ``[TECH-005]``
  * *Status*: ``[x]`` Completed
  * *Unblocks*: Task 4.1.2

* **Task 4.1.2**: Extend ``grm send``, ``grm msg ls``, and ``grm send file`` with ``--topic <topic_id>`` (``message_thread_id``) support.

  * *Delivers*: ``src/cmd_msg.cpp``, ``src/cmd_file.cpp``
  * *Fulfills*: ``[FUNC-008]``
  * *Status*: ``[x]`` Completed
  * *Unblocks*: Phase 4 Completion
