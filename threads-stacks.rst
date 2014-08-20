==================
Threads and Stacks
==================

One unique feature of the µVM is the absence of one-to-one relationship between
stacks and threads. A single thread can swap between multiple stacks to achieve
light-weighted context switch. This can be the underlying support for language
features like co-routines, fibres, light-weighted threads or processes and green threads.

On the other hand, µVM does allow multiple simultaneous threads. µVM threads, by
design, can be implemented as native OS threads and make use of parallel CPU
resources. µVM also defines a memory model. See the `Memory Model
<memory-model>`__ chapter for more information.

This document discusses the µVM threads and µVM stacks.

Concepts
========

µVM supports recursive and nested function calls. A **stack** is the context of
nested or recursive activations of function. A stack has many **frames** each of
which is the context of one function activation. A frame contains the states of
all local values (parameters and instructions) and the program counter which
indicates the instruction to be executed or currently being executed. A frame
also contains memory areas of data allocated by the ``ALLOCA`` or
``ALLOCAHYBRID`` instructions. Because µVM supports `function redefinition
<uvm-client-interface#func-redef>`__, a frame only corresponds to a particular
version of a function and this relation is not changed in the event of function
redefinition.

A **thread** is the unit of CPU scheduling. A thread always runs on a stack
unless paused. The execution on the stack mutates the stack, changing its
values, push or pop frames or allocate memory on the stack, according to the
concrete instructions executed. A stack can have at most one thread on it at any
moment.

A thread can dis-associate from a stack and re-associate with another stack.
This operation is called **swap-stack**. Swap-stack can happen when executing
specific intrinsic functions including ``@uvm.swap_stack`` and
``@uvm.swap_and_kill`` or when the Client is involved during the handling of
exceptional events including traps, undefined function calls and so on.

States of Stacks
================

A stack is in a specific state at any moment. Some operations may change its
state.

READY
    This stack is ready to run, but is not running. This implies that it is
    not associated with any thread. As long as another thread is associated to
    this stack by creating a new thread or swapping to it, it will run.

ACTIVE
    There is a thread running on this stack.

DEAD
    This stack is dead. It cannot be used again and its resources can be
    reclaimed by the garbage collector.

CALLING
    This stack is executing a ``CALL``, ``INVOKE``, ``TAILCALL``, ``NEWSTACK``
    or other instructions or intrinsic functions that involves functions, but
    the callee is not defined. Stacks in this state cannot be simply continued
    as in READY state.

ERROR
    This stack is executing an instruction which encounters an erroneous
    condition, including stack overflow caused by function call or ALLOCA-family
    instructions. Stacks in this state cannot be simply continued as in READY
    state.

Here is a summary of the state and transition of stacks.

======================= =================== =======================
Operation               Current Stack       New/Destination Stack
======================= =================== =======================
create new stack        N/A                 READY
create new thread       N/A                 ACTIVE
@uvm.swap_stack         ACTIVE -> READY     READY -> ACTIVE
@uvm.kill_stack         N/A                 READY -> DEAD
@uvm.swap_and_kill      ACTIVE -> DEAD      READY -> ACTIVE
@uvm.thread_exit        ACTIVE -> DEAD      N/A
TRAP                    ACTIVE -> READY     N/A
undefined function      ACTIVE -> CALLING   N/A
stack-related errors    ACTIVE -> ERROR     N/A
======================= =================== =======================

Stack and Thread Creation
=========================

The Client usually starts a µVM program by creating a new stack containing the
entry point function and then creating a thread on it. The thread, once created,
starts execution immediately.

The µVM provides `API <uvm-client-interface>` for the Client to create new
stacks and new threads. The same thing can be done in µVM programs via the
`NEWSTACK <instruction-set#inst-newstack>`__ instruction and the
`@uvm.new_thread <intrinsic-funcs#ifunc-uvm-new-thread>`__ intrinsic function.

The µVM-Client API is implementation-specific. The following example only shows
a possible implementation::

    MicroVM microVM = ...;
    int funcID = ...;

    Stack s = microVM.newStack(funcID, new Args[] {new IntArg(1), new
                FloatArg(2.0)});
    Thread t = microVM.newThread(s.getID());

The following example does the same thing in the µVM IR::

    .funcdef @callee <void (int<32> float)> (%param0 %param1) {
        ...
    }

    .funcdef @caller <void ()> () {
        %s = NEWSTACK <void (int<32> float)> @callee (1 2.0f)
        %t = ICALL @uvm.new_thread (%s)
        ...
    }

A newly created stack is in the READY state. When a thread is created on it, the
stack enters the ACTIVE state.

Thread Termination
==================

The only way to terminate a thread is to let the µVM program execute the
`@uvm.thread_exit() <intrinsic-funcs#ifunc-uvm-thread-exit>`__ intrinsic
function.  This is invoked by the ``ICALL`` instruction, which is not a basic
block terminator even though ``@uvm.thread_exit`` never returns. The common
practice is to add a throw instruction after it to terminate the basic block.

TODO: An ``UNREACHABLE`` instruction like the ``unreachable`` instruction in
LLVM may be more elegant.

::

    .const @NULLREF <ref<void>> = NULL

    .funcdef @caller <void ()> () {
        ...
    %exit:
        ICALL @thread_exit ()
        THROW @NULLREF  // unreachable
    }

After termination, the stack which the thread runs upon is also killed.

Swap-stack
==========

The `@uvm.swap_stack(%s) <intrinsic-funcs#ifunc-uvm-swap-stack>`__ and the
`@uvm.swap_and_kill(%s) <intrinsic-funcs#ifunc-uvm-swap-and-kill>`__ intrinsic
functions perform the swap-stack operation. Both dis-associate the current
thread from the current stack and re-associates the current thread with another
stack specified by the parameter ``%s`` and transfers the destination stack to
the ACTIVE state. ``@uvm.swap_stack`` transfers the state of the current stack
to the READY state and can continue with the next instruction after this
intrinsic function when another thread is associated with this stack.
``@uvm.swap_and_kill`` will kill the current stack and it cannot be used again.

``@uvm.swap_stack`` cannot pass parameters to another stack. Information has to
be transferred via memory, most likely via ALLOCA. The following example
demonstrates the implementation of a co-routine::

    .const @NULLREF <ref<void>> = NULL
    .typedef @i64 = int<64>

    // This coroutine (a generator) yields 1, 2 and 3 sequentially and stop.
    // param %from: the stack of the main coroutine, i.e. the "caller" stack
    // param %yield: the memory location to write yielded values
    // param %stop: write 1 here to tell the main coroutine "I have stopped".
    .funcdef @one_two_three <void (stack iref<@i64> iref<@i64>)
                            (%from %yield %stop) {
        STORE <@i64> %yield 1
        ICALL @uvm.swap_stack(%from)
        STORE <@i64> %yield 2
        ICALL @uvm.swap_stack(%from)
        STORE <@i64> %yield 3
        ICALL @uvm.swap_stack(%from)
        STORE <@i64> %stop 1
        ICALL @uvm.swap_and_kill(%from)
        THROW @NULLREF  // unreachable
    }

    .funcdef @main <void ()> () {
    %entry:
        // Allocate memory space
        %cur_num_ref = ALLOCA @i64
        %stopped_ref = ALLOCA @i64  // initially 0

        // Create a new stack
        %coro = NEWSTACK <void (iref<@i64> iref<@i64>)> @one_two_three (
                %cur_num_ref %stopped_ref
                )
        BRANCH %head

    %head:
        // Run the coroutine
        ICALL @swap_stack (%coro)

        // See if the coroutine stopped
        %stopped = LOAD <@i64> %stopped_ref
        %cond = EQ <@i64> %stopped 1
        BRANCH2 %cond %exit %body

    %body:
        // If not stopped, read the current yielded value
        %cur_num = LOAD <@i64> %cur_num_ref
        ... do something with %cur_num
        BRANCH %head

    %exit:
        ICALL @thread_exit ()
        THROW @NULLREF  // unreachable
    }

The ``@one_two_three`` function first writes a value into a memory location
passed via the parameter and calls the ``@uvm.swap_stack`` intrinsic function to
go back to the main coroutine. After the last yielding, it writes 1 to the
``%stop`` memory location and uses ``@uvm.swap_and_kill`` to destroy its own
stack. The ``@main`` function repeatedly swap-stack and read from the
``%cur_num_ref`` location until 1 is read from ``%stopped_ref``.

Every time ``@uvm.swap_stack`` is invoked, the thread continues on the target
stack at where it were before. When the new stack was created, it will continue
from the beginning of a function. If a stack discontinued by another
``@uvm.swap_stack`` swapping away from it, it will continue after that
swap-stack.

This is only one of the many ways to implement coroutines. If the client is
smart enough to inline the coroutine into the main function, the performance may
be better than this.

Stack Destruction
=================

The `@uvm.kill_stack(%s) <intrinsic-funcs#ifunc-uvm-kill-stack>`__ and the
previously mentioned `@uvm.swap_and_kill(%s)
<intrinsic-funcs#ifunc-uvm-swap-and-kill>`__ intrinsic functions can destroy a
stack. A destroyed stack enters the DEAD state. It cannot be used again and its
memory can be reclaimed by the garbage collector. The ``@uvm.kill_stack``
intrinsic function can only kill stacks in the READY state.

A stack can be killed by the client, too, via the `µVM-Client Interface
<uvm-client-interface>`__. The client can kill stacks in any state except
ACTIVE.

Stack, Trap and On-stack Replacement
====================================

The `TRAP <instruction-set#inst-trap>`__ instruction and its conditional
variant, the `WATCHPOINT <instruction-set#inst-watchpoint>`__ instruction, can
transfer the control from the µVM program to the Client. These instructions
imply a swap-stack operation to a Client stack, leaving the µVM stack in the
READY state. Returning from the Client-side trap handler performs another
swap-stack operation, but the current thread may be associated to a different
stack than that triggered the trap.

TODO: The current TRAP and WATCHPOINT instructions have return values, which is
inconsistent with the description here. These instructions should be changed not
to have return values.

The µVM is design to support on-stack replacement (OSR). The µVM has enough
information to remove frames from the stack. For this reason, the
``@uvm.swap_stack`` intrinsic function is also an OSR point. The client can
perform OSR on stacks that are paused by swap-stack, which may not be that
triggered the TRAP.

TODO: Really?

Stacks and Exceptions
=====================

The µVM has enough information to unwind any stacks. For this reason, exceptions
can be raised on stacks that are swapped away by ``@uvm.swap_stack`` or TRAP.

TODO: Really?

Frame State Construction
========================

A heavy-weighted mechanism to construct a new frame in arbitrary states (values
of local values and the program counter).

TODO

.. vim: tw=80
