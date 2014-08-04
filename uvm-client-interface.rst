====================
µVM-Client Interface
====================

This page defines the abstract interface between the µVM and the Client,
including messages, call-backs, signals and so on. Because how a µVM is
implemented is not defined by this specification, the concrete interface (i.e.
the concrete API in high-level languages) of particular µVM implementations may
vary. 

Start-up
========

How to start a µVM and/or a client is implementation-specific.

In the beginning, there are no µVM stacks and no µVM threads in the µVM.

µVM IR Code Loading
===================

µVM IR code is provided by the client. It is delivered in the unit of a bundle.
A bundle consists of many type definitions, function signature definitions,
constant definitions, global data definitions, function declarations and
function definitions, which are collectively called top-level definitions. See
`uVM IR <uvm-ir>`__ for more details. 

How a bundle is delivered from the client to the µVM is implementation-specific.

Multiple bundles can be sequentially delivered to the µVM. If the µVM implements
parallel bundle delivery, the result must be equivalent to as if they were
delivered in a specific sequence.

In a bundle, if a types, function signatures, constants, global data or function
declaration has the same ID or name as an existing top-level definition defined
in a previous bundle, it is an error.

If a function definition has the ID as a previous function definition or
function declaration, it must also have the same function signature and the new
function definition **redefines** the previous function definition or
declaration. If the signatures are different, it is an error.

When a function definition redefines another function definition or declaration,
all existing call sites to the previously defined or undefined function now
calls the newly defined function.

TODO: when a µVM thread runs simultaneously with the client loading a bundle, is
the old version of a function still visible to any µVM thread? If so, is the old
version of the function still visible to the client?

Stack and Thread Creation
=========================

TODO

Trap and Undefined Function Handling
====================================

TODO

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

.. vim: tw=80
