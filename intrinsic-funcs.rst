===================
Intrinsic Functions
===================

This document specifies intrinsic functions. Intrinsic functions work the same
as instructions except they all have the same form: all intrinsic functions have
many value parameters, have one return value (may be void), may keep many
registers alive and may throw an exception.

This document uses the ``id : @name :: signature`` notation. ID is used in the
binary form and the name (including "@") is used in the text form. The signature
has the form: ``retty (%paramname0: paramtype0, %paramname1: paramtype1,
...)``, where ``retty`` is the return type, ``%paramnamex`` is the parameter
name and ``paramtypex`` is the parameter type.

Thread and Stack operations
===========================

- ``0x201 : @uvm.new_thread :: thread (%s: stack)``

Create a new thread for a given stack ``%s``. The stack must be in the READY
state before calling. After calling ``@uvm.new_thread``, the stack
enters the ACTIVE state and the new thread starts running immediately.
Return the handle to the newly created thread.

- ``0x202 : @uvm.swap_stack :: void (%s: stack)``

- OSR point (may have KEEPALIVE).
- May throw exception.

Swap the current thread from the current stack to a given stack ``%s`` and
continue executing. The current stack will enter the READY state after
calling. The destination stack must be in the READY state before calling and
will enter the ACTIVE state.

- ``0x203 : @uvm.kill_stack :: void (%s: stack)``

Destroy the given stack ``%s``. The stack ``%s`` must be in the READY state
before calling and will enter the DEAD state.

- ``0x204 : @uvm.swap_and_kill :: void (%s: stack)``

Destroy the current stack and swap to another stack ``%s``.  The current stack
will enter the DEAD state after calling.  The stack ``%s`` must be in the READY
state before calling and will enter the ACTIVE state.

- ``0x205 : @uvm.thread_exit :: void ()``

Stop the current thread and kill the current stack. The current stack will enter
the DEAD state after calling. The current thread stops running.

Math functions
==============

TODO: All functions available in math.h should be available in the µVM.

.. vim: tw=80
