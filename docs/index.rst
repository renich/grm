=================
grm Documentation
=================

**Fast, Native Telegram CLI Client Powered by TDLib & C++23.**

Welcome to the official documentation portal for ``grm``.

Documentation Architecture
--------------------------

.. list-table::
   :widths: 25 45 30
   :header-rows: 1

   * - Guide
     - Purpose & Scope
     - Entrypoint
   * - **Business & Strategy**
     - Strategic vision, ROI goals, and user personas.
     - `Business Specifications <business/spec.rst>`_
   * - **Functional Specifications**
     - Requirements [FUNC-001]..[FUNC-015], behavioral contracts, and interfaces.
     - `Functional Specifications <functional/spec.rst>`_
   * - **Technical Architecture**
     - Specifications [TECH-001]..[TECH-009], TDLib concurrency, and component architectures.
     - `Technical Architecture <technical/spec.rst>`_
   * - **Architecture Decisions (ADRs)**
     - Immutable Architecture Decision Records and trade-off analyses.
     - `Architecture Decisions <adrs/index.rst>`_
   * - **Project Roadmap**
     - Sequenced phase milestones, delivery tracking, and verification matrices.
     - `Project Roadmap <project/roadmap.rst>`_
   * - **User Manual & Guide**
     - CLI syntax, options reference, and interactive usage guide.
     - `User Manual <user/grm.rst>`_

.. toctree::
   :maxdepth: 2
   :caption: Documentation Suite

   business/spec
   functional/spec
   technical/spec
   adrs/index
   project/roadmap
   user/grm
   man/grm.1

Indices and tables
==================

* :ref:`genindex`
* :ref:`search`
