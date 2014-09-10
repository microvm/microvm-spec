============
Memory Model
============

The µVM memory model basically follows the C11 memory model with a few
modifications to make it suitable for the µVM.

Overview
========

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
==========================

The program order in the µVM is a total order while the sequenced-before
relationship in C is a partial order, because there are unspecified order of
evaluations in C.

There is no "atomic type" in µVM. Only operations make a difference between
atomic and non-atomic accesses. Using both atomic and non-atomic operations on
the same memory location is an undefined behaviour in the µVM.

The primitives for atomic accesses and fences are provided by the instruction
set of µVM rather than the library. Mutex locks, however, have to be implemented
on top of this memory model.

There is no ``kill_dependency`` in µVM. There is no direct way to kill
dependency in hardwares and the µVM does not perform most optimisations. The
``kill_dependency`` macro is used to instruct the C compiler that it can stop
tracking the dependencies.

Concepts
========

data value
    A data value is an instance of a µVM type. Data values can (but do not have
    to) be represented as a sequence of bytes.

        NOTE: For example, ``int<1>`` is usually used as conditional flags. In a
        realistic implementation, it may represent a flag in some CPU state
        words.

        NOTE: The use of the word "data" is to disambiguate from "SSA value"
        used in some literature.

SSA variable and evaluation
    An SSA variable holds a data value. It is defined by either a declared
    constant, a global memory reference, a function, a parameter or an
    instruction.
    
    The execution of an instruction is called an **evaluation**. An evaluation
    determines the data value associated to an instruction and this process is
    called **value computation**. Accessing the memory or changing the state of
    the execution environment is called **side effect**. An evaluation may have
    side effects.

        NOTE: Older µVM documents uses the word "SSA value" and LLVM also uses
        "SSA value" informally. They mean the same thing. The use of word "SSA"
        before "variable" is to emphasise that the value is assigned exactly in
        one place, unlike in most programming languages.

    TODO: The definition of data value and SSA variable should be moved to their
    respective chapters.

memory and allocation unit
    There are three kinds of memory in the µVM, namely the **heap memory**, the
    **stack memory** and the **global memory**.
    
    Memory is allocated in their respective **allocation units**.
    
    - An allocation unit in the heap memory is called a **heap object**, or
      **object** when unambiguous. It is created when executing the ``NEW`` or
      ``NEWHYBRID`` instructions or their equivalent Client API.  It is
      destroyed when it becomes unreachable from garbage collection roots.
    - An allocation unit in the stack memory is called an **alloca cell**. It is
      created when executing the ``ALLOCA`` or ``ALLOCAHYBRID`` instruction or
      their equivalent Client API. It is destroyed when the stack frame
      containing it is destroyed.
    - An allocation unit in the global memory is called a **global cell**. One
      global cell is created for every ``.const`` declaration in a bundle
      submitted to the µVM. Global cells are never destroyed.

    The **lifetime** of an allocation unit begins when it is created and ends
    when it is destroyed. When an allocation unit is destroyed and another
    allocation unit occupied the same or overlapping space as the former, they
    are different allocation units.

    When a heap object is moved by the garbage collector, it is still the same
    object.

memory location
    A memory location is a region of data storage in the memory which can
    represent a data value.

    The **lifetime** of a memory location is the same as the allocation unit
    that contains it. Different allocation units contain no common memory
    locations.
    
    When a heap object is moved by the garbage collector, any memory locations
    within it remain the same.

        NOTE: The "object" in the C language is the counterpart of "memory
        location" in µVM. The µVM does not have bit fields and a memory location
        is always an "object" in C's sense. To avoid the ambiguity of "object"
        which means a "heap object" in the µVM context, "memory location" is
        always used when referring to the memory.

        NOTE: SSA variables in the µVM do not have memory locations, even
        though they may be spilled by the register allocator. For this reason,
        local variables that can be updated via reference should be allocated
        using ``ALLOCA``.

load, store and access
    To **load** is to copy data from an SSA variable to the memory. To **store**
    is to copy data from the memory to an SSA variable. To **access** is to load
    or store.

    A load operation **sees** a value if that value is moved to the SSA
    variable.

        NOTE: The concrete instructions are named in capital letters: LOAD and
        STORE. The abstract operations are in lower case: load, store and
        access.

initial value
    The initial value of any memory location is defined as the following,
    according the type of data value the memory location represents:

    * The initial value of ``int`` is 0.
    * The initial value of floating point types is positive zero.
    * The initial value of ``ref``, ``iref``, ``weakref``, ``func``, ``stack``
      and ``thread`` is ``NULL``.
    * The initial value of ``tagref64`` is a floating point number which is
      positive zero.
    * The initial values of all fields or elements in ``struct``, ``array`` and
      the variable part of ``hybrid`` are the initial values according to their
      respective types.

conflict
    Two memory accesses conflict if one stores to a memory location and the
    other loads from or stores to the same location.

thread
    A thread is the unit of CPU scheduling. In this memory model, a thread is
    not limited to a µVM thread.

Comparison of Terminology
-------------------------

The following table is a approximate comparison and may not strictly apply.

=================== ============================
C                   µVM
=================== ============================
value               data value
expression          SSA variable
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

    NOTE: Non-µVM code may also perform memory access operations.

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

All accesses that are not NOT_ATOMIC are atomic. Using both non-atomic
operations and atomic operations on the same memory location is an undefined
behaviour.

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

    NOTE: A synchronisation operation can be both acquire and release
    operations.  A fence can be both an acquire and release fence. RELAXED and
    NOT_ATOMIC means not performing synchronisation operations.

TODO: The word "synchronisation operation" is not used elsewhere.

Orders
======

Program Order
-------------

An evaluation A is **sequenced before** another evaluation B if A and B are in
the same thread and A is performed before B. All evaluations performed by a
particular µVM thread together with their "sequenced before" relations form a
total order, called the **program order**.

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

    NOTE: This reflects the mechanisms, including cache coherence, provided by
    some hardware that guarantees such a total order.

A **release sequence** headed by a release operation A on a memory location M is
a maximal contiguous sub-sequence of atomic store operations in the modification
order M, where the first operation is A and every subsequent operation either is
performed by the same thread that performed the release or is an atomic
read-modify-write operation.

    NOTE: In µVM, when a memory location is accessed by both atomic and
    non-atomic operations, it is an undefined behaviour. So the release sequence
    only apply for memory locations only accessed by atomic operations.

    NOTE: Intuitively, there is a invisible fence before a release store (which
    is sometimes actually implemented as this). Seeing a store in the release
    sequence should imply seeing stores before the invisible fence.

The Synchronises With Relation
------------------------------

An evaluation A **synchronises with** another evaluation B if:

- A performs a release operation on object M, and, B performs an acquire
  operation on M, and, sees a value stored by an operation in the release
  sequence headed by A, or
- A is a release fence, and, B is an acquire fence, and, there exist atomic
  operations X and Y, both operating on some memory location M, such that A is
  sequenced before X, X store into M, Y is sequenced before B, and Y sees the
  value written by X or a value written by any store operation in the
  hypothetical release sequence X would head if it were a release operation, or
- A is a release fence, and, B is an atomic operation that performs an acquire
  operation on a memory location M, and, there exists an atomic operation X such
  that A is sequenced before X, X stores into M, and B sees the value written by
  X or a value written by any store operations in the hypothetical release
  sequence X would head if it were a release operation, or
- A is an atomic operation that performs a release operation on M, and, B is an
  acquire fence, and, there exists some atomic operation X on M such that X is
  sequenced before B and sees the value written by A or a value written by any
  side effect in the release sequence headed by A, or
- A is the creation of a thread and B is the beginning of the execution of the
  new thread.

..

    NOTE: A thread can be created by the ``@uvm.new_thread`` intrinsic function
    or its equivalent Client API.

    NOTE: Since there is no explicit heap memory management in µVM, the
    "synchronises with" relation in C involving ``free`` and ``realloc`` does
    not apply in the µVM.

    NOTE: The µVM provides very primitive threading support. The "synchronizes
    with" relations involving ``call_once`` and ``thrd_join`` are not in the
    µVM.

Dependency
----------

An evaluation A **carries a dependency to** another evaluation B, or B *carries
a dependency from* A, if:

- the data value of A is used as a data argument of B unless:

  * the value of A has type void, or
  * A is used in the ``KEEPALIVE`` clause of B, or
  * B is a ``SELECT`` instruction and A is its ``cond`` argument or a is the
    ``iftrue`` or ``iffalse`` argument not selected by ``cond``, or
  * B is a ``PHI`` instruction, and A is not the variable selected by the
    incoming control flow, or
  * A is a comparing or ``INSERTVALUE`` instruction, or
  * B is a ``CALL``, ``INVOKE``, ``EXTRACTVALUE`` or ``CCALL`` instruction, or

- there is a store operation X such that A is sequenced before X and X is
  sequenced before B, and, X stores the value of A to a memory location M, and,
  B is a load operation from M, or
- for some evaluation X, A carries a dependency to X and X carries a dependency
  to B.

..

    NOTE: The "carries a dependency to" relation together with the
    "dependency-ordered before"" relation exploits the fact that some
    processors, notably ARM and POWER, will not reorder load operations if the
    address used in the later in the program order depends on the result of the
    earlier load. On such processors, the earlier load can be implemented as an
    ordinary load without fences and still has "consume" semantic.

    NOTE: Processors including ARM and POWER only respects data dependency, not
    control dependency. The ``SELECT`` instruction and the comparing instruction
    are usually implemented by conditional moves or conditional flags, which
    would end up that the result is control-dependent on the argument rather
    than data dependent.

    NOTE: Operations involving ``struct`` types in the µVM may be implemented as
    no-ops. Consider the following::

        .type @A = struct <int<64> int<64>>
        .const @A_ZERO <@A> = {0 0}

        %v = LOAD CONSUME <int<64>> %some_memory_location
        %x = INSERTVALUE <@A 0> @A_ZERO %v      // {%v 0}
        %y = EXTRACTVALUE <@A 0> %x             // %v
        %z = EXTRACTVALUE <@A 1> %x             // 0

    The µVM can alias ``%y`` with ``%v`` in the machine code, but ``%z`` is
    always a constant zero.

    NOTE: Dependencies may not always be carried across function calls. A
    function may return a constant and it is uncertain if any processor respect
    this order.

An evaluation A is **dependency-ordered before** another evaluation B if any of
the following is true:

* A performs a release operation on a memory location M, and, in another thread,
  B performs a consume operation on M and sees a value stored by any store
  operations in the release sequence headed by A.
* For some evaluation X, A is dependency-ordered before X and X carries a
  dependency to B.

..

    NOTE: The "dependency-ordered before" relation consists of a release/consume
    pair followed by zero or more "carries a dependency to" relations. If the
    consume sees the value of (or "later than") the release operation, then
    subsequent loads that depends on the consume operation should also see
    values stored before the release operation.

The Happens Before Relation
---------------------------

An evaluation A **inter-thread happens before** an evaluation B if A
synchronises with B, A is dependency-ordered before B, or, for some evaluation
X:

* A synchronises with X and X is sequenced before B,
* A is sequenced before X and X inter-thread happens before B, or
* A inter-thread happens before X and X inter-thread happens before B.

..

    NOTE: This basically allows any concatenations of "synchronises with",
    "dependency-ordered before" and "sequenced before" relations, but disallows
    ending with a "dependency-ordered before" relation followed by a "sequenced
    before" relation. It is disallowed because the consume load in the
    "dependency-ordered before" relation only respects later loads that works
    with a location that depends on the consume load, not arbitrary loads
    sequenced after it. It is only disallowed in the end because the release
    operation in a "synchronises with" relation or a "dependency-ordered before"
    relation will force the order between it and any preceding operations.

    NOTE: A sequence of purely "sequenced before" is not "inter-thread" and is
    also not allowed in the "inter-thread happens before" relation.

An evaluation A **happens before** an evaluation B if A is sequenced before B or
A inter-thread happens before B.

Value Visibility
----------------

A load operation B from a memory location M shall see the initial value of M,
the value stored by a store operation A sequenced before B, or other permitted
values defined later.

A **visible store operation** A to a memory location M with respect to a load
operation B from M satisfies the conditions:

* A happens before B, and
* there is no other store operation X to M such that A happens before X and X
  happens before B.

A non-atomic load operation B from memory location M shall see the value stored
by the visible store operation A.

    NOTE: If there is ambiguity about which store operation is visible to a
    non-atomic load operation, then there is a data race and the behaviour is
    undefined.

The **visible seqeunce of atomic store operations** to a memory location M with
respect to an atomic load operation B from M, is a maximal contiguous
sub-sequence of atomic store operations in the modification order of M, where
the first operation is visible with respect to B, and for every subsequent
operation, it is not the case that B happens before it. The atomic load
operation B sees the value stored by some atomic load operation in the visible
sequence M. Furthermore, if an atomic load operation A from memory location M
happens before an atomic load operation B from M, and A sees a value stored by
an atomic store operation X, then the value B sees shall either equal the value
seen by A, or be the value stored by an atomic store operation Y, where Y
follows X in the modification order of M.

    NOTE: This means, a load cannot see the value stored by an operation happens
    after it, or a store operation separated by another store in the
    happen-before relation.  Furthermore, the later operation of two loads
    cannot see an earlier value than that seen by the first load.

The execution of a program contains a **data race** if it contains two
conflicting non-atomic memory accesses in different threads, neither happens
before the other. Any such data race results in undefined behaviour.

    NOTE: Using both atomic and non-atomic accesses on the same memory location
    is already an undefined behaviour, whether in the same thread of not.

Special Rules for SEQ_CST
=========================

There shall be a single total order S on all SEQ_CST operations, consistent with
the "happens before" order and modification orders for all affected memory
locations, such that each SEQ_CST load operation B from memory location M sees
one of the following values:

* the result of the last store operation A that precedes B in S, if it exists,
  or
* if A exists, the result of some store operation to M in the visible sequence
  of atomic store operations with respect to B that is not SEQ_CST and does not
  happen before A, or
* if A does not exist, the result of some store operation to M in the visible
  sequence of atomic store operations with respect to B that is not SEQ_CST.

For an atomic load operation B from a memory location M, if there is a SEQ_CST
fence X sequenced before B, then B observes either the last SEQ_CST store
operation of M preceding X in the total order S or a later store operation of M
in its modification order.

For atomic operations A and B on a memory location M, where A stores into M and
B loads from M, if there is a SEQ_CST fence X such that A is sequenced before X
and B follows X in S, then B observes either the effect of A or a later store
operation of M in its modification order.

For atomic operations A and B on a memory location M, where A stores into M and
B loads from M, if there are SEQ_CST fences X and Y such that A is sequenced
before X, Y is sequenced before B and X precedes Y in S, then B observes either
the effect of A or a later store operation of M in its modification order.

Special Rules for Atomic Read-modify-write Operations
=====================================================

Atomic read-modify-write operations shall always see the last value (in the
modification order) stored before the store operation associated with the
read-modify-write operation.

Out-of-thin-air or Speculative stores
=====================================

TODO

.. vim: tw=80
