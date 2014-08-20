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

.. _ifunc-uvm-new-thread:

- ``0x201 : @uvm.new_thread :: thread (%s: stack)``

Create a new thread for a given stack ``%s``. The stack must be in the READY
state before calling. After calling ``@uvm.new_thread``, the stack
enters the ACTIVE state and the new thread starts running immediately.
Return the handle to the newly created thread.

.. _ifunc-uvm-swap-stack:

- ``0x202 : @uvm.swap_stack :: void (%s: stack)``

- OSR point (may have KEEPALIVE).
- May throw exception.

Swap the current thread from the current stack to a given stack ``%s`` and
continue executing. The current stack will enter the READY state after
calling. The destination stack must be in the READY state before calling and
will enter the ACTIVE state.

.. _ifunc-uvm-kill-stack:

- ``0x203 : @uvm.kill_stack :: void (%s: stack)``

Destroy the given stack ``%s``. The stack ``%s`` must be in the READY state
before calling and will enter the DEAD state.

.. _ifunc-uvm-swap-and-kill:

- ``0x204 : @uvm.swap_and_kill :: void (%s: stack)``

Destroy the current stack and swap to another stack ``%s``.  The current stack
will enter the DEAD state after calling.  The stack ``%s`` must be in the READY
state before calling and will enter the ACTIVE state.

.. _ifunc-uvm-thread-exit:

- ``0x205 : @uvm.thread_exit :: void ()``

Stop the current thread and kill the current stack. The current stack will enter
the DEAD state after calling. The current thread stops running.

64-bit Tagged Reference
=======================

- ``0x211 : @uvm.tr64.is_fp :: int<1> (%tr: tagref64)``

Test if a ``tagref64`` value contains a floating point value. 1 for true, 0 for
false.

- ``0x212 : @uvm.tr64.is_int :: int<1> (%tr: tagref64)``

Test if a ``tagref64`` value contains an integer value. 1 for true, 0 for false.

- ``0x213 : @uvm.tr64.is_ref :: int<1> (%tr: tagref64)``

Test if a ``tagref64`` value contains an object reference value. 1 for true, 0
for false.

- ``0x214 : @uvm.tr64.from_fp :: tagref64 (%val: double)``

Construct a ``tagref64`` value from a floating point value.

- ``0x215 : @uvm.tr64.from_int :: tagref64 (%val: int<52>)``

Construct a ``tagref64`` value from an integer value.

- ``0x216 : @uvm.tr64.from_ref :: tagref64 (%ref: ref<void>, %tag: int<6>)``

Construct a ``tagref64`` value from an object reference and a tag.

- ``0x217 : @uvm.tr64.to_fp :: double (%tr: tagref64)``

Extract the floating point value from a ``tagref64``, assuming the ``tagref64``
contains floating point value.

- ``0x218 : @uvm.tr64.to_int :: int<52> (%tr: tagref64)``

Extract the integer value from a ``tagref64``, assuming the ``tagref64``
contains integer.

- ``0x219 : @uvm.tr64.to_ref :: ref<void> (%tr: tagref64)``

Extract the object reference from a ``tagref64``, assuming the ``tagref64``
contains reference.

- ``0x21a : @uvm.tr64.to_ref :: int<6> (%tr: tagref64)``

Extract the ``int<6>`` tag accompanying the object reference from a
``tagref64``, assuming the ``tagref64`` contains reference.

Math functions
==============

TODO: All functions available in math.h should be available in the µVM.

.. vim: tw=80
