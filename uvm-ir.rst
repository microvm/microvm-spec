===============================
µVM Intermediate Representation
===============================

µVM Intermediate Representation is µVM's language to representation of a
program.  It is the input from the client and will be executed on the µVM,
either by interpretation or JIT compiling. Ahead-of-time compiling is also
possible.

µVM IR itself has three representations: a text form for human to read, a binary
form for clients to generate µVM IR programs in a compact way and µVM's internal
form of inter-referenced objects.

This document describes the text form. For the binary form, see
:doc:`uvm-ir-binary`.

For the documents of the type system and the instruction set, see:

- :doc:`type-system`
- :doc:`instruction-set`

Example
=======

Here is an example of µVM IR in the text form::

    .const @answer <int<64>> = 42

    .global @some_global_data <struct <int<64> double ref<void>>>

    .typedef @Node = struct<int<64>, @NodeRef>
    .typedef @NodeRef = ref<@Node>

    .funcsig @BinaryFunc = int<64> (int<64> int<64>)

    .funcdecl @square_sum <int<64> (int<64> int<64>)>

    .funcdef @gcd <int<64> (int<64> int<64>)> (%a0 %b0) {
        %entry:
            BRANCH %head

        %head:
            %a = PHI <int<64>> { %entry: %a0; %body: %b; }
            %b = PHI <int<64>> { %entry: %b0; %body: %b1; }
            %z = EQ <int<64>> %b 0
            BRANCH2 %z %exit %body

        %body:
            %b1 = SREM <int<64>> %a %b
            BRANCH %head

        %exit:
            RET <int<64>> %a
    }

Top-level Structure
===================

A **bundle** is the unit of code the client sends to the µVM. It contains many
*definitions*.

For Java users, it is the counterpart of a Java class file in the text form.

It may define many *types*, *function signatures*, *constants*, *global data*
and *functions*. It may also *declare functions* without defining them.

Identifiers
-----------

In the µVM IR text form, things can be given names. There are global identifiers
and local identifiers.

Global identifiers begin with ``@`` and can be used for top-level definitions,
including types, function signatures, constants, global data and functions.
Example::

    .typedef    @i64 = int<64>
    .funcsig    @main_sig = int<32> (int<32> iref<iref<int<8>>>)
    .const      @zero <int<64>> = 0
    .global     @errno <int<64>>
    .funcdecl   @main <@main_sig>

Local identifiers begin with ``%``. They are used only within function
definitions and are only valid within the function definitions in which they are
defined. It can be used for local SSA Values (instructions) and labels.
Example::

    .funcdef @main <@main_sig> (%argc %argv) {
            %rv = ADD <int<32>> %argc 1         // Instruction are local
            BRANCH %bb1
        %bb1:                       // Labels are local
            RET <int<32>> %rv
    }

Type Definition
===============

µVM provides a simple but expressive type system.

A type is constructed by a finite but potentially recursive combination of type
constructors, including :uvm:type:`int`, :uvm:type:`float`, :uvm:type:`double`,
:uvm:type:`ref`, :uvm:type:`iref`, :uvm:type:`weakref`, :uvm:type:`struct`,
:uvm:type:`array`, :uvm:type:`hybrid`, :uvm:type:`void`, :uvm:type:`func`,
:uvm:type:`thread`, :uvm:type:`stack` and :uvm:type:`tagref64`. They are
documented in :doc:`type-system`.

In the text form, wherever a type is expected, it can be written inline using
the above constructors or give a name to a type and reference that type by name.

A type definition gives a name to a type. It looks like::

    .typedef NAME = TYPECTOR

where ``NAME`` is a global identifier for the name and ``TYPECTOR`` is a type
constructor which defines the type.

A type definition can be as simple as this::

    .typedef @i64 = int<64>

It gives a name ``@i64`` to a 64-bit integer.

Sometimes it is necessary to use type definitions because the types may be
recursive::

    .typedef @Node = struct<int<64> @NodeRef>
    .typedef @NodeRef = ref<@Node>

These defines a node in a singly-linked list. The second field of the struct is
an object reference to itself. Note that **the order of top-level definitions
does not matter**. They can be written in any order.

There is no way to simply make an alias of another type. ``.typedef @Foo =
@Bar`` is illegal because ``@Bar`` is not a type constructor. In this event,
replacing all occurrences of ``@Foo`` with ``@Bar`` in the whole program is the
desired approach.

Constant Definition
===================

A constant definition has the form::

    .const NAME <TYPE> = VALUE

where ``NAME`` is global identifier for the constant, ``TYPE`` is a type and
``VALUE`` is its value.

Four kinds of constants can be defined in the text form, namely integers,
floating-point numbers, structs and null values.

- Integer constants are constructed by integer literals, which can be decimal,
  octal (begin with 0) or hexadecimal (begin with 0x). Integer constants must
  have integer types, i.e. ``int<n>``.
- Floating point constants are constructed by floating point literals, which
  must contain exactly one decimal point and may have an exponent part, like
  "12.345e67" or "1.0". Floating point constants must have floating point types,
  i.e. ``float`` and ``double``.
- Struct constants are constructed by struct literals which are an
  open brace ``{`` followed by a sequence of constant expressions (may be
  identifiers) separated by spaces and ends with a closing brace ``}``. Struct
  constants must have type ``struct<...>``.
- Null constants are constructed by the null literal, which is ``NULL``. Null
  constants must have reference types or opaque types referring to µVM objects
  (these include :uvm:type:`ref`, :uvm:type:`iref`, :uvm:type:`weakref`,
  :uvm:type:`func`, :uvm:type:`thread` and :uvm:type:`stack`). The ``NULL``
  value is the value that does not refer to anything. Other values of these
  types cannot be defined as constants, i.e. it is impossible to make a const
  reference to a heap object. If such references are needed, ``.global`` is a
  possible alternative.

Example::

    .typedef @i64 = int<64>
    .const @answer <@i64> = 42
    .const @pi_approx <double> = 3.14159
    .const @some_record <struct<
            int<64> @i64 float double struct<
                int<8> int<16> int<32>>>> = {1 2 3.0 4.0 {5 6 7}}
    .const @other_record <struct<@i64 double>> = {@answer @pi_approx}

    .const @null_ref <ref<void>> = NULL
    .const @null_iref <iref<void>> = NULL
    .const @null_weakref <weakref<void>> = NULL
    .const @null_func <func<void ()>> = NULL
    .const @null_thread <thread> = NULL
    .const @null_stack <stack> = NULL


After definition, the identifier will be bound to an `SSA Value <ssa-value_>`_
of a constant.

The ``.const`` directive cannot define constants of other types. However, other
directives (``.global``, ``.funcdef`` and ``.funcdecl``) can implicitly define
constant SSA Values, too.

Global Data
===========

Global data are memory data allocated when a bundle is loaded. They are similar
to static or global variables in C or C++. The global data are considered part
of the garbage collection roots. They are precisely scanned, which means the GC
can identify all references in global data. Unlike the heap, once allocated,
global data will not be reclaimed by the garbage collector unless the client
deallocate such memory cells using µVM implementation-specific methods. 

Global data are initialised to 0 (0 or 0.0 for integers or FP numbers, or NULL
for reference or opaque types. All fields are zero for composite types). The
client can read or write global data in an implementation-specific way. Doing so
before program runs can initialise them to meaningful values.

A global data definition has the form::

    .global NAME <TYPE>

where ``NAME`` is a global identifier and ``TYPE`` is the *referent* type.

The identifier of global data defined by ``.global`` represents a constant SSA
Value of type ``iref<T>``, an :uvm:type:`iref` (internal reference) to the
global data whose type is ``T``. They can be used by :uvm:inst:`LOAD`,
:uvm:inst:`STORE` and other memory operating instructions.

For example::

    .global @my_errno <int<32>>
    .global @hello_world_str <array<int<8> 12>>

    .funcdef @func <...> (...) {
        %a = LOAD <int<32>> @my_errno       // @my_errno has type iref<int<32>>
        STORE <int<32>> @my_errno 0
        ...
    }

Function Definition and Declaration
===================================

Functions represent executable code in the µVM.

Functions can be **defined** in µVM using ``.funcdef``::

    .funcdef NAME <SIG> (PARAMS) { BODY }

where ``NAME`` is a global identifier; ``SIG`` is a function signature;
``PARAMS`` is the formal parameter list and ``BODY`` is a sequence of
instructions, constants and labels.

A signature is a return value plus a parameter list, in the form ``RV (P1 P2 P3
...)``, where ``RV``, ``P1``, ``P2``, ... are types of the return value and the
parameters, respectively. For example, a function with two parameters, a float
and a double, and a return value of ``int<64>`` can be defined as::

    .funcdef @example <int<64> (float double)> (%p0 %p1) {
        ...
    }

For convenience, the signature can be defined separately using ``.funcsig``::

    .funcsig NAME = RV (P1 P2 P3 ...)

where ``NAME`` is a global identifier. The signature of the above ``@example``
function can be defined separately as::

    .funcsig @ExampleSig = int<64> (float double)

    .funcdef @example <@ExampleSig> (%p0 %p1) {
        ...
    }

The formal parameter list is a list of local identifiers, each of which receives
the value of one parameter passed to the function. The number of formal
parameters must match the signature.

A function can be **declared** without defining using ``.funcdecl``::

    .funcdecl NAME <SIG>

where ``NAME`` is a global identifier and ``SIG`` is the signature.

A declared function has no body and can be defined later.

Note that the definitions does not have an order. It is allowed to define two
functions that call each other without having to declare the second 

A function can be re-defined provided that the signature is not changed. The new
function will replace the old one and all existing call sites to the old
function will automatically call the new version.

The identifier of a function defined by ``.funcdef`` or ``.funcdecl`` represents
a constant SSA Value of type :uvm:type:`func`. It can be used by the
:uvm:inst:`CALL`, :uvm:inst:`INVOKE`, :uvm:inst:`TAILCALL` and
:uvm:inst:`NEWSTACK` instructions.


Function Identifier
-------------------

Each function, declared or defined, has a unique function identifier, which is
**not** the identifier in the text form or the binary form of the µVM IR. It is
the value of the :uvm:type:`func` type, which is opaque in the sense that the
underlying binary runtime representation is an implementation detail of the µVM.
It may be implemented as the address of the compiled code, but does not have to
be.

When a function is declared, such a unique ID is reserved for the function. When
defining a function, the function ID is bound to the definition. When
re-defining a function, the newly defined function body replaces the older
version, but the function ID does not change. All existing values of the
:uvm:type:`func` type remain valid, but refers to the newer version of the
function, instead. All existing activation of the older version of the function
remain to be valid. Decided by the implementation, the garbage collector may
reclaim the space of compiled function code once there is no active frames of
the older version on all stacks.

Function Body
=============

When defining a function using ``.funcdef``, the body of the function must be
supplied.

The function body consists of many instructions with labels in between marking
the beginnings of basic blocks.

The body of a function consists of many basic blocks.

The first basic block, called the entry block and conventionally named
``%entry``, is the unique entry point of a function.

Each basic block contains a sequence of instructions. The grammar for each
instruction is defined separately in :doc:`instruction-set`. In the text form,
each instruction may optionally have an identifier::

    %rv = OPCODE <type_param> arg1 arg2 ...
    OPCODE <type_param> arg1 arg2 ...

In the above example, ``%rv`` is the identifier of the first instruction. The
identifier represents an SSA Value (introduced later) of the value returned by
the instruction.

Example::

    .funcdef @gcd <int<64> (int<64> int<64>)> (%a0 %b0) {

        %entry:
            BRANCH %head

        %head:
            %a = PHI <int<64>> { %entry: %a0; %body: %b; }
            %b = PHI <int<64>> { %entry: %b0; %body: %b1; }
            %z = EQ <int<64>> %b 0
            BRANCH2 %z %exit %body

        %body:
            %b1 = SREM <int<64>> %a %b
            BRANCH %head

        %exit:
            RET <int<64>> %a
    }


It is allowed to give name to instructions that does not return value. In this
case, the client can refer to this instruction using this identifier, but the
identifier does not represent a value. So ``%br = BRANCH %head`` is still valid.

The label ``%entry`` can be omitted. In fact, it can always be omitted because
**it is not allowed to branch to the entry block**.

In the text form, constants can be written inline, as long as its type can be
inferred from the current instruction. For example. it is allowed to write::

    %z = EQ <int<64>> %b 0

because the type of ``0`` can be inferred from the type parameter ``int<64>``.

The last instruction of any basic block must be a terminating instruction, which
is one of

- :uvm:inst:`BRANCH`: Unconditional branching.
- :uvm:inst:`BRANCH2`: Binary conditional branching.
- :uvm:inst:`SWITCH`: Multi-way branching.
- :uvm:inst:`INVOKE`: Call a function, anticipating an exception to be thrown.
- :uvm:inst:`TAILCALL`: Call a function and return its return value.
- :uvm:inst:`RET`: Return from a function.
- :uvm:inst:`RETVOID`: Return from a function of :uvm:type:`void` return type.
- :uvm:inst:`THROW`: Throw an exception to the caller.
- :uvm:inst:`TRAP`: Transfer the control to the client for further processing.
  Exception may be thrown from the client.
- :uvm:inst:`WATCHPOINT`: A trap that is disabled in the beginning and can be
  enabled asynchronously by the client later during execution.
- :uvm:inst:`IINVOKE`: Call an intrinsic function, anticipating an exception.

Two kinds of instructions must appear (in any order) in the beginning of any
basic blocks except the entry block. They are

- :uvm:inst:`PHI`: The phi-node of the SSA form. Its value depends on the
  incoming control flow.
- :uvm:inst:`LANDINGPAD`: Only used as the exceptional destination of
  :uvm:inst:`INVOKE`, :uvm:inst:`TRAP`, :uvm:inst:`WATCHPOINT` and
  :uvm:inst:`IINVOKE`, which anticipate an exception. Its value is an object
  reference to the exception object (which may be anything since µVM only
  require that exceptions are object references, but does not define the content
  of such objects).

The SSA Form
------------

The µVM IR uses the SSA form. Every instruction operates on SSA Values. An SSA
Value is defined in exactly one **place**. This does not imply that it is only
assigned one *time* since a loop structure will re-evaluate an instruction.

.. _ssa-value:

An SSA Value is one of

- constant: The Value which never changes. Specifically, 

  - The identifier of constants defined by the ``.const`` directive represents
    an SSA Value of the type indicated.

  - In the text form, literal values written inline are treated as constants of
    the type expected by the respective instructions they are used in.

  - The identifier of Global data defined by ``.global`` represents a constant
    SSA Value of type ``iref<T>``, an :uvm:type:`iref` (internal reference) to
    the global data whose type is ``T``.

  - The identifier of a function defined by ``.funcdef`` or ``.funcdecl``
    represents a constant SSA Value of type :uvm:type:`func`.

- instruction: The Value which is computed. Specifically,

  - The identifier of an instruction, if present, represents an SSA Value of its
    return value.

In µVM, parameters are implemented as an instruction implicitly defined by the
formal parameter list. Control flow analysis can treat them as being defined in
the beginning of the entry block.

The Instruction Set
-------------------

The full instruction set can be found in :doc:`instruction-set`.

.. vim: textwidth=80
