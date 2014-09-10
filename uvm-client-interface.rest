====================
µVM-Client Interface
====================

This page defines the abstract interface between the µVM and the Client,
including messages, call-backs, signals and so on. Because how a µVM is
implemented is not defined by this specification, the concrete interface (i.e.
the concrete API in high-level languages) of particular µVM implementations may
vary. 

To reduce confusion, in this article, "message" means a message sent from the
client to the µVM. "call-back" means a message sent from the µVM to the client.
Both are synchronous, means the caller should wait for the callee to finish,
unless explicitly specified as "asynchronous".

How to register those call-backs is implementation-specific. How messages and
call-backs are represented is implementation-specific. How parameters and return
values are represented is implementation-specific.

NOTE: They may be represented as classes, objects, methods, delegates, events,
signals, interfaces, actor messages, functions and so on, depending on whether
the implementation uses object-oriented, actor-based, functional, procedural or
other kinds of programming languages.

A "handle" is something that the client can use to refer to some object in the
µVM. This can be implemented in whatever way convenient to the implementation.

Start-up
========

How to start a µVM and/or a client is implementation-specific.

In the beginning, whether there are existing types, signatures, constants,
global values or functions already defined or declared, or µVM stacks or no µVM
threads already created is implementation-specific.

µVM IR Code Loading
===================

µVM IR code is provided by the client. It is delivered in the unit of bundles.
A bundle consists of many type definitions, function signature definitions,
constant definitions, global data definitions, function declarations and
function definitions, which are collectively called top-level definitions. See
`uVM IR <uvm-ir>`__ for more details. 

How a bundle is delivered from the client to the µVM is implementation-specific.

Multiple bundles can be sequentially delivered to the µVM. If the µVM implements
parallel bundle delivery, the result must be equivalent to as if they were
delivered in a specific sequence.

In a bundle, if a types, function signatures, constants, global data or function
declaration has the same ID or name as any existing top-level definitions
defined in previous bundles, it is an error.

.. _func-redef:

If a function definition has the ID as a previous function definition or
function declaration, it must also have the same function signature and the new
function definition **redefines** the previous function definition or
declaration. If the signatures are different, it is an error.

When a function definition redefines another function definition or declaration,
all existing call sites to the previously defined or undefined function now
calls the newly defined function.

All existing activations of any functions remain unchanged. Only new calls will
activate the new version. IDs of function-local entities, including parameters,
basic blocks and instructions, cannot be reused, hence old IDs of them still
refer to them in the old version of functions. Specifically, traps can still
introspect stack frames of functions of old versions using old IDs of
instructions.

Stack and Thread Creation
=========================

The ``new_stack`` message
-------------------------

In addition to the ``NEWSTACK`` instruction, the MicroVM provides the client a
message ``new_stack`` so that the client can create new stacks.

- name: ``new_stack``
- parameters:

  - ``callee``: The function whose activation will be at the bottom frame of the
    stack.
  - ``args``: The arguments to that function.

- returns: A handle to the new stack

The ``new_thread`` message
--------------------------

In addition to the ``@uvm.new_thread`` intrinsic function, the MicroVM provides
the client a message ``new_thread`` so that the client can create new threads.

- name: ``new_thread``
- parameters:

  - ``stack``: The stack which the new thread is initially bound to.

- returns: A handle to the new thread.

As the ``@uvm.new_thread`` intrinsic function, a newly created thread is started
immediately.

Trap and Undefined Function Handling
====================================

The ``TRAP`` and the ``WATCHPOINT`` instructions represent places that a µVM
program requires assistance from the client. When a µVM program calls a µVM
function that is declared but not defined, the µVM also asks the client for
further actions. In both cases, the µVM pauses the running µVM thread, preserves
the stack state and transfers the control to call-backs registered by the
client.

The ``handle_trap`` call-back
-----------------------------

This call-back is called when a trap (both ``TRAP`` and enabled ``WATCHPOINT``)
is hit. Since there may be multiple µVM threads running, more than one thread
may hit the trap simultaneously. The implementation is only required to have all
of them handled, sequentially or parallelly.

- name: ``handle_trap``
- parameters:

    - ``thread``: A handle to the thread which hit the trap.

- returns: The client should tell the µVM how to resume execution.

TODO: Define an extra ``TRAPPED`` state of a stack.

TODO: The trap handler may dis-associate a thread with its stack and
re-associate with another stack. The state of the new stack is unknown (but a
good client must be sure about its state).
  
.. whether the trap should return
   normally with the appropriate return value, or exceptionally with an object
   reference referring to an object as the exception. See the ``TRAP`` instruction
   for more details. If the client stops the thread, the return value is ignored by
   the µVM. If OSR happened, the return value is also ignored.


The ``thread`` handle can be used to introspect all states of the stack,
including the trap ID which can be obtained as the "current instruction".

The ``handle_undefined_function`` call-back
-------------------------------------------

This call-back is called when an undefined function is called (by ``CALL``,
``INVOKE`` or ``TAILCALL``), used by ``NEWSTACK`` or any intrinsic functions
that may call a µVM function. Since there may be multiple µVM threads running,
more than one thread may call undefined functions simultaneously. The
implementation is only required to have all of them handled, sequentially or
parallelly.

- name: ``handle_undefined_function``
- parameters:

    - ``thread``: A handle to the thread which hit the trap.
    - ``function_id``: The ID of the undefined function.

- returns: nothing

The client should define the undefined function. After returning, the µVM
program will re-execute the same instruction again unless the thread is stopped.

Thread and Stack Introspection
------------------------------

When a trap is hit or an undefined function is called, the thread pauses and the
client takes over the control. It gives the client an opportunity to introspect
the state of the thread and its associated stack.

This specification defines the relations between some µVM objects and some
messages to manipulate them. How those objects are represented, how to navigate
through those objects and how to access their attributes are
implementation-specific. The implementation may represent them as objects in
the implementation language (e.g. Java Objects when the µVM and the Client are
implemented in Java), or just their IDs.

Object ``Thread``

    - attribute ``stack``: its associated stack

Object ``Stack``

    - attribute ``top``: the top frame of the stack
    - attribute ``state``: the state of a stack (READY, RUNNING, DEAD)
      (todo: TRAPPED?)

Object ``Frame``

    - attribute ``next``: the next frame under the current frame if the current
      frame is not the bottom frame of the stack
    - attribute ``cur_inst``: the current (µVM) instruction
    - attribute ``func``: the specific version of a function which is active in
      this frame
    - attribute ``keep_alives``: the values of local SSA Values kept alive by
      the KEEPALIVE clause of the current instruction

The kept-alive values of all frames, not just the top frame, can be
introspected. Since function re-definition is allowed in µVM, a frame is
associated with an exact version of a function, not just the function itself.

TODO: The top frame may be executing any instruction that is allowed in its
state. If 

.. The top frame should be executing a ``TRAP`` or ``WATCHPOINT`` instruction when
    handling a trap, or ``CALL``, ``INVOKE``, ``TAILCALL`` or ``NEWSTACK`` (or
    intrinsic functions introduced in the future) when handling an undefined
    function. All frames below are executing ``CALL`` or ``INVOKE`` (or intrinsic
    functions).

There are only one trap handler and one undefined function handler registered at
a time. The client can multiplex these handlers by inspecting the ID of the
current instruction to know which instruction triggered this trap or undefined
function and handle it accordingly.

On-stack Replacement (OSR)
--------------------------

On-stack replacement is a way to replace existing function activations (i.e.
stack frames) with others.

OSR can only be done on a stack whose top frame is executing an OSR point
instruction.

NOTE: Undefined function calls are not OSR points. ``TRAP``, ``WATCHPOINT`` and
intrinsic functions labelled as OSR points are.

The µVM imposes restrictions on what OSR can do to a stack.

1. It can pop as many frames as desired.
2. After popping, it must push one stack with a function and its arguments. This
   function must have the return value expected by the caller in the frame
   below. When the execution is resumed to this stack, it will start at the
   beginning of this new function.

TODO: Is the above two steps atomic, in the sense that it is a single action:
"pop n frames and push one frame with func and args"? If it is not atomic, what
intermediate state is a stack in? Is it necessary to maintain some intermediate
states for those intermediate frames? (e.g. callee-saved registers?)

TODO: Add a "RETURNING" state? But how to "return a value of a specific type to
a caller that expect this type" in a generic way? In other words, does it
require return-type-specific code? If it does, it is not different from
generating a trivial function per type that returns its argument.

TODO: Is it reasonable enough to limit a new frame to start from the beginning
of a function? It seems to be the only sane solution.

Client-held GC Roots
====================

TODO

Direct Memory Access for the Client
===================================

TODO

Signal Handling
===============

The µVM needs to handle (to be defined) some hardware traps including
divide-by-zero errors and floating point exceptions. These should be implemented
by signal handling in UNIX-like operating systems. Meanwhile the client may also
need to handle such erroneous cases, for example, when implementing an
interpreter. According to how the operating system works, only one signal
handler can be registered by a process at the same time.

In an environment where the µVM is present, the client should not register the
signal handler. The µVM should register the signal handler. When signals arrive,
e.g. SIGFPE for divide-by-zero error, the µVM should check if the error occurs
in any µVM IR code. If so, it should be handled within the µVM (to be defined)
by taking the exceptional branching (to be defined). If it does not occur in any
µVM IR code, it should let the client handle it by calling back or sending
messages to the client depending on the implementation. Errors like
divide-by-zero within the µVM runtime (e.g. the garbage collector) are fatal and
will not be handled. The previous signal handler registered by external
libraries will be preserved by the µVM in case the error does not occur within
the client, either, and needs to be daisy-chained to external libraries.

Theoretically the µVM should be the only entity in the process that registers
signal handlers and all other entities, including the client or other libraries
designed with the µVM in mind, when having the need to handle signals, should
use the call-back or message-passing mechanisms provided by the µVM.

.. vim: tw=80
