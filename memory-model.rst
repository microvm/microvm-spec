============
Memory Model
============

The µVM memory model basically follows the C11 memory model with a few
modifications to make it suitable for the µVM.

Overview
--------

The µVM does not enforce any strong order, but trusts the Client to correctly
use the atomic and ordering mechanisms provided by the µVM. Many choices of
ordering, from relaxed to sequentially consistent, on each memory operation are
given to the Client. The Client has the freedom to make its choice and has the
responsibility to synchronise their multi-threaded programs.

The most restricted form of memory accesses are sequentially consistent. The µVM
guarantees that they are atomic. They follow the well-known acquire-release
memory model and there is a total order of all such memory accesses in all
threads.

A less-restricted form is acquire and release. It does not have the total
order provided by sequential consistency, but atomicity and the synchronise-with
relationship between release and acquire operations are provided.

The "consume" order exploits the fact that some processors with relaxed memory
order can figure out the dependencies between load operations in the hardware
and will not reorder them even without memory fences. It can achieve some
synchronisation requirements more efficiently than the "acquire" order.

The "relaxed" order only guarantees atomicity but does not enforce any order.
The most unrestricted form of memory access is not atomic. These operations
allows the µVM implementation and the processor to maximise the throughput while
relying on the programmer to correctly synchronise their programs.

Notable differences from C
--------------------------

The program order in the µVM is a total order while the sequenced-before
relationship in C is a partial order, because there are unspecified order of
evaluations in C.

There is no "atomic type" in µVM. Only operations make a difference between
atomic and non-atomic accesses.

The primitives for atomic accesses and fences are provided by the instruction
set of µVM rather than the library. Mutex locks, however, have to be implemented
on top of this memory model.

The presence of garbage collection, especially copying garbage collection,
introduces differences. The definition of memory location is different and the
initial values is more complex in the µVM.

There is no ``kill_dependency`` in µVM. There is no direct way to kill
dependency in hardwares and the µVM does not perform most optimisations. The
``kill_dependency`` macro is used to instruct the C compiler that it can stop
tracking the dependencies.

Concepts
========

data value
    A data value is an instance of a µVM type. Data values can (but do not have
    to be) represented as a sequence of bytes.

    NOTE: ``int<1>`` is usually used as conditional flags. In a realistic
    implementation, it may represent a flag in some CPU state words.

SSA Value
    An SSA Value is a computation that is defined in exactly one place
    (instructions), or something that has a data value which can be used in such
    computations (declared constants, global data references, functions and
    parameters).
    
    The execution of an instruction is called an **evaluation**. An evaluation
    determines the data value associated to an instruction and this process is
    called **value computation**. Accessing the memory or changing the state of
    the execution environment is called **side effect**. An evaluation may have
    side effects.

memory
    There are three kinds of memory in the µVM, namely the **heap memory**, the
    **stack memory** and the **global memory**. An allocation unit in the heap
    memory is called a **heap object**, or **object** when unambiguous.

memory location
    A memory location is a region of data storage in the memory which can
    represent a data value. When the memory location is in the heap memory, it
    is part of a heap object. When the heap object is moved by the garbage
    collector, the memory location remains to be the same part of the object.

    NOTE: The "object" in the C language is the counterpart of "memory location"
    in µVM. The µVM does not have bit fields and a memory location is always an
    "object" in C's sense. To avoid the ambiguity of "object" which means a
    "heap object" in the µVM context, "memory location" is always used when
    referring to the memory.

    NOTE: The µVM clearly distinguishes memory and SSA Values. SSA Values do not
    have memory locations even though register allocators may spill them on the
    stack. This means if a local variable in a programming language can be
    access as if it is in the memory, it must be implemented as memory (heap,
    stack or global).

load, store and access
    To load is to move data from an SSA Value to the memory. To store is to move
    data from the memory to an SSA Value. To access is to load or store.

    NOTE: The concrete instructions are named in capital letters: LOAD and
    STORE.

conflict
    Two memory accesses conflict if one stores to a memory location and the
    other loads from or stores to the same location.

Comparison of Terminology
-------------------------

The following table is a approximate comparison and may not strictly apply.

=================== ============================
C                   µVM
=================== ============================
value               data value
expression          SSA Value
object              memory location
(N/A)               object
read                load
modify              store
=================== ============================

Memory Operations
=================

The following µVM instructions access their specified memory location: LOAD,
STORE, CMPXCHG and ATOMICRMW. Specifically,

- LOAD performs a load operation.
- STORE performs a store operation.
- CMPXCHG performs a load operation. It also performs a store operation on
  success.
- ATOMICRMW performs both a load and a store operation.

The FENCE instruction is a fence.

Memory accessing instructions and fences have memory orders, which are the
following:

- NOT_ATOMIC
- RELAXED
- CONSUME
- ACQUIRE
- RELEASE
- ACQ_REL (acquire and release)
- SEQ_CST (sequentially consistent)

TODO: fix the `instruction set <instruction-set>`_ page.

All accesses that are not NOT_ATOMIC are atomic.

- LOAD shall have NOT_ATOMIC, RELAXED, CONSUME, ACQUIRE or SEQ_CST order.
- STORE shall have NOT_ATOMIC, RELAXED, RELEASE or SEQ_CST order.
- CMPXCHG shall have RELAXED, ACQUIRE, RELEASE, ACQ_REL or SEQ_CST on success
  and RELAXED, ACQUIRE or SEQ_CST on failure.
- ATOMICRMW shall have RELAXED, ACQUIRE, RELEASE, ACQ_REL or SEQ_CST order.
- FENCE shall have ACQUIRE, RELEASE, ACQ_REL or SEQ_CST order.

=========== ======= ======= =============== =============== =========== =====
Order       LOAD    STORE   CMPXCHG(succ)   CMPXCHG(fail)   ATOMICRMW   FENCE
=========== ======= ======= =============== =============== =========== =====
NOT_ATOMIC  yes     yes     no              no              no          no
RELAXED     yes     yes     yes             yes             yes         no
CONSUME     yes     no      no              no              no          no
ACQUIRE     yes     no      yes             yes             yes         yes
RELEASE     no      yes     yes             no              yes         yes
ACQ_REL     no      no      yes             no              yes         yes
SEQ_CST     yes     yes     yes             yes             yes         yes
=========== ======= ======= =============== =============== =========== =====

- A load operation with ACQUIRE, ACQ_REL or SEQ_CST order performs a **acquire**
  operation on its specified memory location.
- A load operation with CONSUME order performs a **consume** operation on its
  specified memory location.
- A store operation with RELEASE, ACQ_REL or SEQ_CST order performs a
  **release** operation on its specified memory location.
- A fence with ACQUIRE, ACQ_REL or SEQ_CST order is a **acquire fence**.
- A fence with RELEASE, ACQ_REL or SEQ_CST order is a **release fence**.

Acquire operation, consume operation, release operation and fences are
collectively called **synchronisation operations**.

Both CMPXCHG and ATOMICRMW are **atomic read-modify-write operations**.

NOTE: A synchronisation operation can be both acquire and release operations.
A fence can be both an acquire and release fence. RELAXED and NOT_ATOMIC means
not performing synchronisation operations.

Orders
======

Program Order
-------------

An evaluation A is **sequenced before** another evaluation B if A and B are in
the same thread and A is performed before B. All evaluations performed by a
particular thread form a total order, called the **program order**.

NOTE: In C, there is only a partial order because of unspecified order of
evaluations.

Modification Order
------------------

All atomic store operations on a particular memory location M occur in some
particular total order, called the **modification order** of M. If A and B are
atomic stores on memory location M, and A happens before B, then A shall precede
B in the modification order of M.

NOTE: This is to say, the modification order is consistent with the happens
before order.

A **release sequence** headed by a release operation A on a memory location M is
a maximal contiguous sub-sequence of atomic store operations in the modification
order M, where the first operation is A and every subsequent operation either is
performed by the same thread that performed the release or is an atomic
read-modify-write operation.

TODO: There is no "atomic type" in µVM. The case when a memory location is
accessed by both atomic and non-atomic operations needs more description.

The Synchronises With Relation
------------------------------

TODO: Define the "synchronises with" relation, including all cases.

Dependency
----------

TODO: Define the "carries a dependency to" and "dependency-order before"
relations.


The Happens Before Relation
---------------------------

TODO: Define the "inter-thread happens before" and "happens before" relation.

Value Visibility
----------------

TODO: Define "visible stores", "store seen by a load", "visible sequence of
stores" and the case of data race.

NOTE: All memory accesses in µVM are explicit. Avoid using "value of" or "side
effect" as C implicitly load/store while evaluating expressions.

.. vim: tw=80
