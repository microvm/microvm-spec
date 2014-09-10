===============
Instruction Set
===============

Special Non-operation Instructions
==================================

Label
-----

In the text form, it is like::

    %label_name:

that is, a local identifier followed by a colon ``:``.

In the binary form, it has a special pseudo-instruction:

+------+
| opct |
+======+
| 0xF9 |
+------+

The presence of a label (or the pseudo instruction) indicates the beginning of a
basic block in a function definition.

The label for the first basic block in a function may be omitted. It is
conventionally called ``%entry``, but this convention is not enforced.

For LLVM users: it has labels, too.

Basic Operations
================

These operations are the very basic of the µVM.

Binary Operations
-----------------

- ``binOp < T > op1 op2````

    ``binOp``
        The binary operation. One in the table shown later.
    ``T``
        *type*: The type of both operands.
    ``op1``, ``op2``
        *value* of ``T``: Two operands.
    return value
        Type ``T``: return the result of the computation.

+--------+-----+-----+-----+
| opct   | idt | idt | idt |
+========+=====+=====+=====+
| opcode | T   | op1 | op2 |
+--------+-----+-----+-----+

``binOp`` is one in the following table:

========= ======== ===== ======================== ========== ==========
 Keyword   opcode   T     semantic                 op1        op2
========= ======== ===== ======================== ========== ==========
 ADD       0x01     int   add                      neutral    neutral
 SUB       0x02     int   subtract                 neutral    neutral
 MUL       0x03     int   multiply                 neutral    neutral
 SDIV      0x04     int   signed divide            signed     signed
 SREM      0x05     int   signed remainder         signed     signed
 UDIV      0x06     int   unsigned divide          unsigned   unsigned
 UREM      0x07     int   unsigned remainder       unsigned   unsigned
 SHL       0x08     int   left shift               neutral    unsigned
 LSHR      0x09     int   logical right shift      unsigned   unsigned
 ASHR      0x0A     int   arithmetic right shift   signed     unsigned
 AND       0x0B     int   bit-wise and             neutral    neutral
 OR        0x0C     int   bit-wise or              neutral    neutral
 XOR       0x0D     int   bit-wise exclusive or    neutral    neutral
 FADD      0xB0     FP    FP add                   FP         FP
 FSUB      0xB1     FP    FP subtract              FP         FP
 FMUL      0xB2     FP    FP multiply              FP         FP
 FDIV      0xB3     FP    FP divide                FP         FP
 FREM      0xB4     FP    FP remainder             FP         FP
========= ======== ===== ======================== ========== ==========

This family of instructions are simple arithmetic and logical operations.
Operations started with "F" are for floating point numbers and others are for
integers. Both ``op1`` and ``op2`` must have type ``T``. The requirement of the
type ``T`` is listed in the above table.

Most integer arithmetic or logical instructions (``ADD``, ``SUB``, ``MUL``,
``AND``, ``OR`` and ``XOR``) work for both unsigned and signed integers since
negative integers are in the 2's complement representation. Others treat
operands as signed or unsigned integers.

TODO: specify overflow, divide by zero and the semantic of each instruction.

For LLVM users: this is directly borrowed from LLVM. Vector operations are
currently not supported and will be added in the (near) future.

Example::

    .const @a <int<32>> = 42
    .const @x <double> = 42.0d

    %b = ADD <int<32>> @a 1
    %c = SUB <int<32>> @a %b

    %y = FADD <double> @x 1.0d
    %z = FSUB <double> @x %y

Comparison
----------

- ``cmpOp < T > op1 op2``

    ``cmpOp``
        The comparison operation. One in the table shown later.
    ``T``
        *type*: The type of both operands.
    ``op1``, ``op2``
        *value* of ``T``: Two operands.
    return value
        Type ``int<1>``: returns 1 for true or 0 for false.

+--------+-----+-----+-----+
| opct   | idt | idt | idt |
+========+=====+=====+=====+
| opcode | T   | op1 | op2 |
+--------+-----+-----+-----+

``cmpOp`` is one in the following table:

========= ======== =======================
 Keyword   opcode   T
========= ======== =======================
 EQ        0x20     int or EQ-comparable
 NE        0x21     int or EQ-comparable
 SGE       0x22     int
 SGT       0x23     int
 SLE       0x24     int
 SLT       0x25     int
 UGE       0x26     int
 UGT       0x27     int
 ULE       0x28     int
 ULT       0x29     int
 FFALSE    0xC0     FP
 FTRUE     0xC1     FP
 FUNO      0xC2     FP
 FUEQ      0xC3     FP
 FUNE      0xC4     FP
 FUGT      0xC5     FP
 FUGE      0xC6     FP
 FULT      0xC7     FP
 FULE      0xC8     FP
 FORD      0xC9     FP
 FOEQ      0xCA     FP
 FONE      0xCB     FP
 FOGT      0xCC     FP
 FOGE      0xCD     FP
 FOLT      0xCE     FP
 FOLE      0xCF     FP
========= ======== =======================

This family of instructions are simple comparison operations.  Operations
started with "F" are for floating point numbers and others are for integers. The
type ``T`` must be ``float``, ``double`` or ``int<...>`` according to the
operator and both ``op1`` and ``op2`` must have type ``T``.

See LLVM's LangRef for the semantic of each operator.

TODO: the semantics will eventually be written in this document.

The ``EQ`` and the ``NE`` operator are also applicable to ``ref``, ``iref``,
``func``, ``stack``, ``thread`` (which are collectively called "EQ-comparable")
and the result is whether they refer to the same object, field, function, stack
or thread, respectively. In the case of ``tagref64``, it checks for exact
bit-wise equality, which may not be desired because both operands can contain
floating point numbers.

The return value has type ``int<1>``. 1 for true and 0 for false.

For LLVM users: this is directly borrowed from LLVM. Vector operations are
currently not supported and will be added in the (near) future.

Example::

    .const @a <int<32>> = 42
    .const @b <int<32>> = 43
    %c = GT <int<32>> @a 0
    %d = LT <int<32>> @a @b

    .const @w <double> = 42.0d
    .const @x <double> = 43.0d
    %y = FULT <double> @w 0.0d
    %z = FUGT <double> @w @x

    %e = NEW <int<64>>
    %f = EQ <ref<int<64>>> %e %e

    %g = ALLOCA <int<64>>
    %h = EQ <iref<int<64>>> %g %g

Conversion
----------

- ``convOp < T1 T2 > opnd``

    ``convOp``
        The conversion operation. One in the table shown later.
    ``T1``, ``T2``
        *type*: The source type and the destination type, respectively.
    ``opnd``
        *value* of ``T1``: The operand.
    return value
        Type ``T2``: The result of the conversion.

+--------+-----+-----+-----+
| opct   | idt | idt | idt |
+========+=====+=====+=====+
| opcode | T1  | T2  | op  |
+--------+-----+-----+-----+

``convOp`` is one in the following table.

========== ======== ========= =========
 Keyword    opcode   T1        T2
========== ======== ========= =========
 TRUNC      0x30     int       int    
 ZEXT       0x31     int       int    
 SEXT       0x32     int       int    
 FPTRUNC    0x33     FP        FP
 FPEXT      0x34     FP        FP
 FPTOUI     0x35     FP        int    
 FPTOSI     0x36     FP        int    
 UITOFP     0x37     int       FP
 SITOFP     0x38     int       FP
 BITCAST    0x39     int/FP    FP/int
 REFCAST    0x3A     ref       ref
 IREFCAST   0x3B     iref      iref
 FUNCCAST   0x3C     func      func
========== ======== ========= =========

This family of instructions convert one primitive type to another or adjust the
length of numbers. ``T1`` and ``T2`` are the source type and the destination
type, respectively and the operand ``opnd`` must match the type ``T1``.

``REFCAST`` casts between object reference types ``ref<T>``; ``IREFCAST`` casts
between internal reference types ``iref<T>``; ``FUNCCAST`` casts between
function types ``func<sig>``. They only changes the referent type or the
function signature and does not change the value of the reference or the
referred function and does not perform any kind of runtime type checking.

See LLVM's documentation for the semantic of other operators.

For LLVM users: These instructions are borrowed from LLVM. µVM currently lacks
the conversion between raw pointer types and numerical types and they will be
added when raw pointers are introduce. ``REFCAST``, ``IREFCAST`` and
``FUNCCAST`` are µVM-specific. µVM cannot use ``bitcast`` to cast between
reference types.

Example::

    .const @a  <int<32>> = 42
    .const @a2 <int<32>> = -42
    %b  = TRUNC <int<32> int<16>> @a    // is int<16>
    %c  = ZEXT <int<32> int<64>> @a     // is int<64>
    %c2 = SEXT <int<32> int<64>> @a2    // is int<64>
    %d  = UITOFP <int<32> double> @a    // is double
    %d2 = SITOFP <int<32> double> @a2   // is double

    .const @x  <double> = 42.0d
    %y = FPTRUNC <double float> @x      // is float
    %z = FPEXT <float double> %y        // is double
    %w = FPTOSI <double int<64>> @x     // is int<64>

    %f = NEW <@Foo>                         // is ref<@Foo>
    %g = REFCAST <ref<@Foo> ref<@Bar>> %f   // is ref<@Bar>

    %h = ALLOCA <@Foo>                      // is ref<@Foo>
    %i = IREFCAST <ref<@Foo> iref<@Bar>> %h // is ref<@Bar>

    .funcsig @vv = void ()
    .funcsig @main_sig = int<32> (int<32> iref<iref<int<8>>>)
    .funcdecl @j <@main_sig>
    %k = FUNCCAST <@main_sig @vv> @j    // is func<@vv>

Selection (a.k.a. Conditional Move)
-----------------------------------

- ``SELECT < T > cond iftrue iffalse``

    ``T``
        *type*: The type of ``iftrue``, ``iffalse`` and the result.
    ``cond``
        *value* of ``int<1>``: The condition
    ``iftrue``, ``iffalse``
        *value* of ``T``: The result for this instruction when ``cond`` is 1 or
        0, respectively.
    return value
        Type ``T``: ``iftrue`` or ``iffalse`` according to ``cond``

+------+-----+-------+--------+---------+
| opct | idt | idt   | idt    | idt     |
+======+=====+=======+========+=========+
| 0x40 | T   | cond  | iftrue | iffalse |
+------+-----+-------+--------+---------+

The ``SELECT`` instruction is a conditional instruction. When ``cond``
is 1, the value of this ``SELECT`` is the value of ``iftrue``, otherwise
``iffalse``. This will translate to the concrete architecture's conditional move
instructions (e.g. the CMOV for x86).

For LLVM users: This instruction is inspired by LLVM's ``select`` instruction.

Example::

    .const @a <int<64>> = 42
    %a_mod_2    = SREM <int<64>> @a 2
    %a_is_even  = EQ <int<64>> %a_mod_2 0
    %b = SELECT <int<64>> %a_is_even 100 200    // %b is 100 if %a is even
                                                // 200 otherwise

Intra-function Control Flow
===========================

The following instructions are for jumping within a function.

One-way (unconditional) Branch
------------------------------

- ``BRANCH dest``

    ``dest``
        *label*: The destination of jumping.

+------+------+
| opct | idt  |
+======+======+
| 0x90 | dest |
+------+------+

``BRANCH`` transfers the control flow to the beginning of a basic block,
identified by the label ``dest``.

For LLVM users: This is the same as the one-branch ``br`` instruction.

Example::

    %entry:
        BRANCH %head

    %head:
        // Continue executing here.

Two-way Branch
--------------

- ``BRANCH2 cond iftrue iffalse``

    ``cond``
        *value* of ``int<1>``: The condition
    ``iftrue``, ``iffalse``
        *label*: The destination to jump to when ``cond`` is 1 or 0,
        respectively

+------+------+--------+---------+
| opct | idt  | idt    | idt     |
+======+======+========+=========+
| 0x91 | cond | iftrue | iffalse |
+------+------+--------+---------+

``BRANCH2`` branches based on a conditional value, usually computed by a
comparison instruction. If cond is 1, jump to ``iftrue``, otherwise jump to
``iffalse``.

For LLVM users: This is the same as the two-branch ``br`` instruction.

Example::

    .const @a <int<64>> = ...

    %entry:
        %b = EQ <int<64>> @a 42
        BRANCH2 %b %equal %notequal

    %equal:
        // if %b is 1, jump here

    %notequal:
        // if %b is 0, jump here

Multi-way Branch (Switching)
----------------------------

- ``SWITCH < T > opnd default { case1 : dest1 ; case2 : dest2 ; ...  }``

    ``T``
        *type*: The type of ``opnd`` and ``casex``
    ``opnd``
        *value* of ``T``: The value to compare against.
    ``default``:
        *label*: The default destination. i.e. The destination if no case
        matches.
    ``casex``:
        *constant value* of ``T``: The case value for a branch.
    ``destx``:
        *label*: The destination for the corresponding case.

+------+-----+------+---------+--------+-------+-------+-----+
| opct | idt | idt  | idt     | lent   | idt   | idt   | ... |
+======+=====+======+=========+========+=======+=======+=====+
| 0x92 | T   | opnd | default | ncases | casex | destx | ... |
+------+-----+------+---------+--------+-------+-------+-----+

``SWITCH`` compares the value of ``opnd`` against each case. If value
equals ``casex``, then branch to ``destx``. If none matches, branch to
``default``.

In the binary form, ncases is the number of cases and there are this number of
case-target pairs following.

For LLVM users: This is the same as the ``switch`` instruction.

Example::

    .const @a <int<64>> = ...

    %entry:
        SWITCH <int<64>> @a %defbranch {
            1: %one;
            2: %two;
            3: %three;
            }

    %defbranch:
        ...

    %one:
        ...

    %two:
        ...
        
    %three:
        ...

Phi-node
--------

- ``PHI < T > { bb1 : val1 ; bb2 : val2 ; ...  }``

    ``T``
        *type*: The type of this instruction and ``valx``.
    ``bbx``
        *label*: The basic block where the control flow comes from.
    ``valx``
        *value* of ``T``: The value of the current instruction for the
        corresponding control flow source.
    return value
        Type ``T``: One of ``valx`` according to the control flow.

+------+-----+------+-----+------+-----+
| opct | idt | lent | idt | idt  | ... |
+======+=====+======+=====+======+=====+
| 0x93 | T   | nbbs | bbx | valx | ... |
+------+-----+------+-----+------+-----+

The value of the ``PHI`` instruction depends on the incoming control
flow. If the control flow comes from ``bbx`` then the value is ``valx``. The set
of all ``bbx`` must exhaust all possible branches to the current basic block.

The ``PHI`` instruction must appear in the beginning of a basic block.
Multiple ``PHI`` instructions are allowed.

In the binary format, ``nbbs`` is the number of ``bbx`` and there are this
number of bb-val pair following.

For LLVM users: This is the same as the ``phi`` instruction.

Example::

    %bb1:
        ...
        %x1 = ADD <int<64>> ... ...
        BRANCH %bbfoo

    %bb2:
        ...
        %x2 = SUB <int<64>> ... ...
        BRANCH %bbfoo

    %bb3:
        ...
        %x3 = MUL <int<64>> ... ...
        BRANCH %bbfoo

    %bbfoo:
        %x = PHI <int<64>> {
            %bb1: %x1;
            %bb2: %x2;
            %bb3: %x3;
            }
        ...

Inter-function Control Flow
===========================

Function calling
----------------

- ``CALL < sig > func ( arg1 arg2 ... )``
- ``CALL < sig > func ( arg1 arg2 ... ) KEEPALIVE ( v1 v2 ... )``
- ``INVOKE < sig > func ( arg1 arg2 ... ) nor exc``
- ``INVOKE < sig > func ( arg1 arg2 ... ) nor exc KEEPALIVE ( v1 v2 ... )``
- ``TAILCALL < sig > func ( arg1 arg2 ... )``

    ``sig``
        *function signature*: The signature of the callee.
    ``func``
        *value* of ``func``: The callee.
    ``argx``
        *value* according to ``sig``: Parameter.
    ``nor``, ``exc``
        *label*: The normal destination and exceptional destination
    ``vx``
        *value* of any type: Registers to be retained on the stack frame.
    return value
        The return type of ``sig``: The return value of the callee. The
        ``TAILCALL`` instruction itself does not have a value.

CALL:

+------+-----+------+-------+------+------+-----+------+-----+-----+-----+
| opct | idt | idt  | lent  | idt  | idt  | ... | lent | idt | idt | ... |
+======+=====+======+=======+======+======+=====+======+=====+=====+=====+
| 0x60 | sig | func | nargs | arg1 | arg2 | ... | nka  | v1  | v2  | ... |
+------+-----+------+-------+------+------+-----+------+-----+-----+-----+

INVOKE:

+------+-----+------+-----+-----+-------+------+------+-----+------+-----+-----+-----+
| opct | idt | idt  | idt | idt | lent  | idt  | idt  | ... | lent | idt | idt | ... |
+======+=====+======+=====+=====+=======+======+======+=====+======+=====+=====+=====+
| 0x61 | sig | func | nor | exc | nargs | arg1 | arg2 | ... | nka  | v1  | v2  | ... |
+------+-----+------+-----+-----+-------+------+------+-----+------+-----+-----+-----+

TAILCALL:

+------+-----+------+-------+------+------+-----+
| opct | idt | idt  | lent  | idt  | idt  | ... |
+======+=====+======+=======+======+======+=====+
| 0x62 | sig | func | nargs | arg1 | arg2 | ... |
+------+-----+------+-------+------+------+-----+

``CALL`` and ``INVOKE`` creates a new stack frame for the
callee, passes the arguments and starts executing from the callee's entry block.
The value of ``CALL`` and ``INVOKE`` is the return value of the
callee.

After the callee normally returned, ``CALL`` continues executing the
next instruction after this ``CALL``. If the callee throws an exception,
it is rethrown to the caller of the current function.

``INVOKE`` expects exception to be thrown from the callee. If the callee
returns normally, then branch to the basic block identified by label ``nor``. If
an exception is thrown from the callee, then branch to basic block ``exc`` where
a ``LANDINGPAD`` instruction will receive the exception thrown.

``TAILCALL`` is semantically similar to calling a function and
immediately return the returned value. The ``TAILCALL`` instruction
replaces the caller's stack frame with that for the callee. So
``TAILCALL`` requires that the callee must have the same return type as
the caller. The caller of the current function becomes the caller of the
function ``TAILCALL`` calls. Normal or abrupt returning are handled by
the new pair of caller of callee.

To facilitate stack introspection and on-stack replacement (OSR), some local SSA
variables (parameters and instructions) can be explicitly kept alive at the call
site. Exactly these variables appeared in the KEEPALIVE list (``v1``, ``v2``,
...) can be introspected from the client or via stack introspection instructions
which may be introduced in the future.

For LLVM users:

- The meaning of ``TAILCALL`` is similar to LLVM's ``musttail``: in µVM,
  a ``TAILCALL`` always replaces the current stack frame.
- Calling conventions cannot be specified in µVM: µVM always uses its internal
  calling conventions.  
- Arguments will not be automatically zero or sign-extended or truncated for the
  code. Conversions must be explicitly done before calling.  
- The ``func`` type in µVM is a dedicated function identifier, not a pointer.  
- All parameters are passed by value and parameters are SSA Values. To pass
  on-stack data or arrays, use ``alloca`` and pass ``iref``.  
- Local SSA Variable retention is unique to µVM.

Example::

    .funcdecl @sum <double (double double)>

    .funcdef @square_sum <double (double double)> (%x %y) {
        %entry:
            %x2 = MUL <double> %x %x
            %y2 = MUL <double> %y %y

            // return the result of sum(x2,y2)
            TAILCALL <double (double double)> @sum (%x2 %y2)    
    }

    .funcsig @dddsig double (double double)

    .funcdef @main <void ()> () {
        %entry:
            %a = CALL <@dddsig> @square_sum (3.0d, 4.0d)
            %b = INVOKE <@dddsig> @square_sum (%a, 5.0d) %nor %exc

        %nor:
            %c = CALL <@dddsig> @square_sum (5.0d, 6.0d) KEEPALIVE (%a)
            %d = INVOKE <@dddsig> @square_sum (%c, %c) %nor2 %exc KEEPALIVE (%a %c)

        %nor2:
            // continue here
        
        %exc:
            %the_exception = LANDINGPAD
            // handle the exception
    }

Normal Returning from Functions
-------------------------------

- ``RET < T > rv``

    ``T``
        *type*: The return type.
    ``rv``
        *value* of ``T``: The return value.

- ``RETVOID``

RET:

+------+-----+-----+
| opct | idt | idt |
+======+=====+=====+
| 0x63 | T   | rv  |
+------+-----+-----+

RETVOID:

+------+
| opct |
+======+
| 0x64 |
+------+

``RET`` normally returns from the current function with ``rv`` as the
return value. ``RETVOID`` normally returns from the current function
whose return type is void.

For LLVM users: Equivalent to LLVM's ``ret`` and ``ret void``.

Example::

    .funcdef @sum <double (double double)> (%x %y) {
        %s = ADD <double> %x %y
        RET <double> %s
    }

    .funcdef @main <void ()> () {
        RETVOID
    }

Abrupt Returning
----------------

- ``THROW exc``

    ``exc``
        *value* of ``ref`` to any object: The exception object.

+------+-----+
| opct | idt |
+======+=====+
| 0x65 | exc |
+------+-----+

``THROW`` abruptly returns from the current function and throws
exception ``exc``. Exceptions in µVM are object references to any type. This
exception is caught by the nearest caller which uses ``INVOKE`` and the
``exc`` value is captured by the ``LANDINGPAD`` instruction of its
exceptional destination.

For LLVM users: There is no equivalent in LLVM. The ``resume`` instruction in
LLVM continues the propagation of a in-flight exception. This can also be done
by µVM's ``THROW`` instruction. µVM programs can create a new exception
object by ``NEW`` and throw it, where LLVM must depend on
platform-specific libraries to allocate new exceptions.

Example::

    .funcdef @safe_divide <int<64> (int<64> int<64>)> (%x %y) {
        %y0 = EQ <int<64>> %y 0
        BRANCH %y0 %divbyzero %okay

    %divbyzero:
        %exc = NEW <@SomeExceptionType>    // user-defined exception type
        // initialise %exc
        THROW %exc

    %okay:
        // continue 
    }

Receiving the Exception
-----------------------

- ``LANDINGPAD``

    return value
        Type ``ref<void>``: The exception.

+------+
| opct |
+======+
| 0x66 |
+------+

Used in the beginning of the exceptional destination basic block of an
``INVOKE`` instruction to receive the thrown exception.

The value has type ``ref<void>`` and can be cast to a reference of other
concrete types.

For LLVM users: Unlike LLVM, exceptions is handled internally in µVM. The type
of exceptions in µVM is always an object reference. This is general enough to
handle all cases. Unlike C++ which uses RTTI to identify the exception type, µVM
client must implement its own RTTI, probably in the Java style.

Example: See ``INVOKE``

Aggregate Type Operations
=========================

These instructions operate on the ``struct`` type as SSA Values.

Extract Value
-------------

- ``EXTRACTVALUE < T index > opnd``

    ``T``
        *type*, subtype of struct: The type of the operand.
    ``index``
        *intImmediate*: The index of the field to extract.
    ``opnd``
        *value* of ``T``: The operand.
    return value
        Type is the index-th field of struct type ``T``: The field of opnd.

+------+-----+-------+------+
| opct | idt | lent  | idt  |
+======+=====+=======+======+
| 0x70 | T   | index | opnd |
+------+-----+-------+------+

``EXTRACTVALUE`` extracts the index-th field from an SSA Value ``opnd``
which has type ``struct``.

For LLVM users: It is the counterpart of the ``extractvalue`` instruction in
LLVM. But µVM's ``EXTRACTVALUE`` does not work on arrays or nested
``struct``.  Use ``EXTRACTVALUE`` multiple times to extract the
field in nested structs.

Example::

    .typedef @Foo = struct <int<32> double float>
    .const @s <@Foo> = {42 84.0d 3.14f}

    %a = EXTRACTVALUE <@Foo 1> @s
    // %a == 84.0d and %a is double

    .typedef @Bar = struct <double struct <double double> double>
    .const @t <@Foo> = {1.0d {2.1d 2.2d} 3.0d}

    %b = EXTRACTVALUE <@Bar 1> @t
    // %b == {2.1d 2.2d} and %b is struct <double double>

    %c = EXTRACTVALUE <struct <double double> 0> %b
    // %c == 2.1d and %c is double

Insert Value
------------

- ``INSERTVALUE < T index > opnd newval``

    ``T``
        *type*, subtype of struct: The type of the operand.
    ``index``
        *intImmediate*: The index of the field to insert.
    ``opnd``
        *value* of ``T``: The operand.
    ``newval``
        *value* of the ``index-th`` field of ``T``: The new value for the field.
    return value
        Type ``T``: A new struct with the ``index-th`` field different.

+------+-----+-------+------+--------+
| opct | idt | lent  | idt  | idt    |
+======+=====+=======+======+========+
| 0x71 | T   | index | opnd | newval |
+------+-----+-------+------+--------+

``INSERTVALUE`` creates a new SSA Value which is the same as ``opnd``
except the field indicated by ``index`` which is changed to ``newval``.

For LLVM users: It is the counterpart of the ``insertvalue`` instruction in
LLVM. But µVM's ``INSERTVALUE`` does not work on arrays or nested
``struct``.  Use a combination of ``EXTRACTVALUE`` and
``INSERTVALUE`` to replace a field in a nested struct.

Example::

    .typedef @Foo = struct <int<32> double float>
    .const @s <@Foo> = {42 84.0d 3.14f}

    %a = INSERTVALUE <@Foo 1> @s, 126.0d
    // %a == {42 126.0d 3.14f} and %a is @Foo

    .typedef @Bar = struct <double struct <double double> double>
    .const @t <@Bar> = {1.0d {2.1d 2.2d} 3.0d}

    %b = EXTRACTVALUE <@Bar 1> @t
    // %b == {2.1d 2.2d} and %b is struct <double double>

    %c = INSERTVALUE <struct <double double> 0> %b 999.9d
    // %c == {999.9d 2.2d} and %c is struct <double double>

    %d = INSERTVALUE <@Bar 1> @t %c
    // %d == {1.0d {999.9d 2.2d} 3.0d} and %d is @Bar

Memory Operations
=================

Memory allocation
-----------------

This family of instructions allocate memory on the heap or the stack.

- ``NEW < T >``

    ``T``
        *type*, not ``hybrid``: the type to allocate
    return value
        Type ``ref<T>``: an object reference newly allocated object.

+------+-----+
| opct | idt |
+======+=====+
| 0x10 | T   |
+------+-----+

Allocate a fixed-size object on the heap which has type ``T``. Return the
reference to it.

For LLVM users: There is no equivalence in LLVM.

Example::

    %r = NEW <int<64>>          // ref<int<64>>
    
    .typedef @Foo = struct <int<64> double double>
    %s = NEW <@Foo>              // ref<Foo>

- ``NEWHYBRID < T > length``

    ``T``
        *type*, must be ``hybrid``: the type to allocate
    ``length``
        Type ``int<WORD_LENGTH>``: the length of the variable part of the
        hybrid.
    return value
        Type ``ref<T>``: an object reference newly allocated object.

+------+-----+--------+
| opct | idt | idt    |
+======+=====+========+
| 0x11 | T   | length |
+------+-----+--------+

Allocate an object of the ``hybrid`` type on the heap. ``length`` is the
number of cells in the variable part.

For LLVM users: There is no equivalence in LLVM.

Example::

    .typedef @Foo = hybrid <struct<int<64> double> int<8>>
    %r = NEWHYBRID <@Foo> 16         // ref<@Foo>
    // %r refers to a heap object which has a struct header followed by 16
    // int<8>

- ``ALLOCA < T >``

    ``T``
        *type*, not ``hybrid``: the type to allocate
    return value
        Type ``iref<T>``: an internal reference newly allocated alloca cell.

+------+-----+
| opct | idt |
+======+=====+
| 0x12 | T   |
+------+-----+

Allocate a fixed-size object on the stack which has type ``T``. Return the
internal reference to it.

For LLVM users: Alloca is the closest approximation, but µVM's
``ALLOCA`` allocates exactly one alloca cell. Use
``ALLOCAHYBRID`` to allocate contiguous cells of identical data
structures.

Example::

    %r = ALLOCA <int<64>>           // iref<int<64>>
    
    .typedef @Foo = struct <int<64> double double>
    %s = ALLOCA <@Foo>               // iref<@Foo>

- ``ALLOCAHYBRID < T > length``

    ``T``
        *type*, must be ``hybrid``: the type to allocate
    ``length``
        Type ``int<WORD_LENGTH>``: the length of the variable part of the
        hybrid.
    return value
        Type ``ref<T>``: an object reference newly allocated alloca cell.

+------+-----+--------+
| opct | idt | idt    |
+======+=====+========+
| 0x13 | T   | length |
+------+-----+--------+

Allocate an object of the ``hybrid`` type on the stack. ``length`` is
the number of cells in the variable part.

For LLVM users: This is similar to ``alloca`` with a number of elements. A
``hybrid`` in µVM allows to have a fixed prefix. This is similar to
typical C99 code like ``struct Something { ...; char suffix[];};``.

Example::

    .typedef @Foo = hybrid <struct <int<64> double> int<8>>
    %r = ALLOCAHYBRID <@Foo> 16      // iref<@Foo>

Memory Addressing
-----------------

This family of instructions manipulates references, but does not actually read
or write memory.

- ``GETIREF < T > opnd``

    ``T``
        *type*: The type the operand refers to.
    ``opnd``
        *value* of ``ref<T>``: The operand.
    return value
        Type ``iref<T>``: The internal reference to the same heap object.

+------+-----+--------+
| opct | idt | idt    |
+======+=====+========+
| 0x14 | T   | opnd   |
+------+-----+--------+

This instruction converts an object reference to an internal reference. This is
the first step to access a field of a heap object from an object reference.

For LLVM users: No equivalence.

Example::

    %a = NEW <double>                           // ref<double>
    %b = GETIREF <double> %a                    // iref<double>

- ``GETFIELDIREF < T index > opnd``

    ``T``
        *type*, must be ``struct``: The type the operand refers to.
    ``index``
        *intImmediate*: The index of the field.
    ``opnd``
        *value* of ``iref<T>``: The operand.
    return value
        Type ``iref<U>`` where ``T = struct<... U ...>``: The internal reference
        to the ``index-th`` field in the referenced struct.

+------+-----+--------+--------+
| opct | idt | lent   | idt    |
+======+=====+========+========+
| 0x15 | T   | index  | opnd   |
+------+-----+--------+--------+

``GETFIELDIREF`` gets the internal reference to a field of a
``struct``, given the internal reference to the ``struct``
itself and the index of the field.

For LLVM users: It is similar to the ``getelementptr`` addressing into a struct.

Example::

    .typedef @Foo = struct<double float int<32>>
    %a = ALLOCA <@Foo>
    %b = GETFIELDIREF <@Foo 1> %a               // iref<float>

- ``GETELEMIREF < T > opnd index``

    ``T``
        *type*, must be ``array``: The type the operand refers to.
    ``opnd``
        *value* of ``iref<T>``: The operand.
    ``index``
        *value* of ``int<WORD_LENGTH>`` The index of the element.
    return value
        Type ``iref<U>`` where ``T = array<U>``: The internal reference to the
        ``index-th`` element in the referenced array.

+------+-----+--------+--------+
| opct | idt | idt    | idt    |
+======+=====+========+========+
| 0x16 | T   | opnd   | index  |
+------+-----+--------+--------+

``GETELEMIREF`` gets the internal reference to an element of an
``array``, given the internal reference to the ``array`` itself
and the index of the element.

This does not work on the variable part of a ``hybrid``. Use
``SHIFTIREF``, instead.

For LLVM users: It is similar to the ``getelementptr`` indexing into an array.

Example::

    .typedef @Foo = array<float 100>
    .const @ind <int<64>> = 50                  // assume 64-bit architecture
    %a = ALLOCA <@Foo>
    %b = GETELEMIREF <@Foo> %a @ind             // iref<float>

- ``SHIFTIREF < T > opnd offset``

    ``T``
        *type*, must be ``array``: The type the operand refers to.
    ``opnd``
        *value* of ``iref<T>``: The operand.
    ``offset``
        *value* of ``int<WORD_LENGTH>`` The offset to move.
    return value
        Type ``iref<T>``: The internal reference of ``opnd`` moved forward for
        ``offset`` instances of the referenced data.

+------+-----+--------+--------+
| opct | idt | idt    | idt    |
+======+=====+========+========+
| 0x17 | T   | opnd   | offset |
+------+-----+--------+--------+

``SHIFTIREF`` moves an internal reference forward by ``offset``
instances of the referenced data. It is an alternative way to
``GETELEMIREF`` to address elements in an array. It also works on the
variable part of a ``hybrid``.

``offset`` can be negative, in which case it moves the reference backwards.

For LLVM users: It is similar to the ``getelementptr`` with its first index
being non-zero. It is also like the pointer arithmetic in C which adds an
integer to a pointer.

Example::

    .typedef @Foo = array<float 100>
    .const @ind <int<64>> = 50                  // assume 64-bit architecture
    %a = ALLOCA <@Foo>
    %b = GETELEMIREF <@Foo> %a 0                // iref<float>
    %c = SHIFTIREF <float> %b @ind              // iref<float>

- ``GETFIXEDPARTIREF < T > opnd``
- ``GETVARPARTIREF < T > opnd``

    ``T``
        *type*, must be ``hybrid``: The type the operand refers to.
    ``opnd``
        *value* of ``iref<T>``: The operand.
    return value
        If ``T = hybrid<F V>``, then:

        - ``GETFIXEDPARTIREF``: Type ``iref<F>``: The internal reference
          of the fixed part of ``T``.
        - ``GETVARPARTIREF``: Type ``iref<V>``: The internal reference
          of the first element in the variable part of ``T``.

GETFIXEDPARTIREF:

+------+-----+------+
| opct | idt | idt  |
+======+=====+======+
| 0x18 | T   | opnd |
+------+-----+------+

GETVARPARTIREF:

+------+-----+------+
| opct | idt | idt  |
+======+=====+======+
| 0x19 | T   | opnd |
+------+-----+------+

``GETFIXEDPARTIREF`` and ``GETVARPARTIREF`` get the internal
reference to the fixed part and an internal reference to the first element in
the variable part of the given hybrid, respectively.

To access further elements in the variable part, use the ``SHIFTIREF``
instruction to move forward.

For LLVM users: No equivalent.

Example::

    .typedef @Foo = hybrid<double int<8>>
    .const @ind <int<64>> = 50                  // assume 64-bit architecture
    %a = ALLOCAHYBRID <@Foo> 100
    %f = GETFIXEDPARTIREF <@Foo> %a             // iref<double>
    %v = GETVARPARTIREF <@Foo> %a               // iref<int<8>>
    %v50 = SHIFTIREF <int<8>> %v @ind           // iref<int<8>>

Memory Reading and Writing
--------------------------

This family of instructions actually read or write into the memory.

Note about ``weakref``: Loading from ``iref<weakref<T>>`` produces a
strong ``ref<T>``. Storing a ``ref<T>`` into an ``iref<weakref<T>>`` makes the
reference in the memory weak. CmpXchg does both loading and storing as this.

All instructions in this family can take an optional **atomic ordering** as
shown in the table below. These specify the requirement for atomicity and memory
ordering.

=========== ======  =======================
Keyword     Binary  Semantic
=========== ======  =======================
NOT_ATOMIC  0x00    not atomic
UNORDERED   0x01    unordered
MONOTONIC   0x02    monotonic
ACQUIRE     0x03    acquire
RELEASE     0x04    release
ACQ_REL     0x05    acquire+release
SEQ_CST     0x06    sequentially consistent
=========== ======  =======================

See `<memory-model>`__ for more details.

- ``LOAD < T > loc``
- ``LOAD atomicOrd < T > loc``

    ``atomicOrd``
        *atomic ordering*: requirement for atomicity and memory ordering.
    ``T``
        *type*: The type ``loc`` refers to.
    ``loc``
        *value* of ``iref<T>``: The reference to load from.
    return value
        Type ``T``: The data loaded from the memory. If ``T`` is ``weakref<U>``,
        then the return type is ``ref<U>`` instead of ``weakref<U>``.

+------+------------+-----+------+
| opct | opct       | idt | idt  |
+======+============+=====+======+
| 0x1A | atomicOrd  | T   | loc  |
+------+------------+-----+------+

Load from the memory location referred by ``loc``.

if ``atomicOrd`` is omitted in the text form, it defaults to ``NOT_ATOMIC``.

For LLVM users: This is similar to the ``load`` instruction. ``volatile`` is
absent in µVM because µVM never remove reachable memory load/store instructions.

Example::

    .typedef @Foo = struct<int<64> double float>
    %r = NEW <@Foo>                         // ref<@Foo>
    %i = GETIREF <@Foo> %r                  // iref<@Foo>
    %i1 = GETFIELDIREF <@Foo 1> %i          // iref<double>
    %d = LOAD <double> %i1                  // double

    // Note: all allocations initialise the memory to zero.

- ``STORE < T > loc newval``
- ``STORE atomicOrd < T > loc newval``

    ``atomicOrd``
        *atomic ordering*: requirement for atomicity and memory ordering.
    ``T``
        *type*: The type ``loc`` refers to.
    ``loc``
        *value* of ``iref<T>``: The reference to store into.
    ``newval``
        *value* of ``T``: The new value to store.  If ``T`` is ``weakref<U>``,
        then ``newval`` has type ``ref<U>`` instead of ``weakref<U>``.

+------+------------+-----+------+--------+
| opct | opct       | idt | idt  | idt    |
+======+============+=====+======+========+
| 0x1B | atomicOrd  | T   | loc  | newval |
+------+------------+-----+------+--------+

Store ``newval`` into the memory location referred by ``loc``.

if ``atomicOrd`` is omitted in the text form, it defaults to ``NOT_ATOMIC``.

For LLVM users: This is similar to the ``store`` instruction. ``volatile`` is
also not supported.

Example::

    .typedef @Foo = struct<int<64> double float>
    %r = NEW <@Foo>                         // ref<@Foo>
    %i = GETIREF <@Foo> %r                  // iref<@Foo>
    %i1 = GETFIELDIREF <@Foo 1> %i          // iref<double>
    STORE <double> %i1 3.14159d

- ``CMPXCHG succOrd failOrd < T > loc expected desired``

    ``succOrd``, ``failOrd``
        *atomic ordering*: memory ordering when this operation is successful
        and when failed, respectively.
    ``T``
        *type*: The type ``loc`` refers to.
    ``loc``
        *value* of ``iref<T>``: The reference to the location to load and store.
    ``expected``
        *value* of ``T``: The expected value in the memory.
    ``desired``
        *value* of ``T``: The new value to store into the memory.
    return value:
        Type ``T``: The original value in the memory.
    NOTE:
        If ``T`` is ``weakref<U>``, then ``expected``, ``desired`` and the
        return value have type ``ref<U>`` instead of ``weakref<U>``.

+------+---------+---------+-----+------+----------+---------+
| opct | opct    | opct    | idt | idt  | idt      | idt     |
+======+=========+=========+=====+======+==========+=========+
| 0x1C | succOrd | failOrd | T   | loc  | expected | desired |
+------+---------+---------+-----+------+----------+---------+

Store ``desired`` into the memory location referred by ``loc`` if and only if
the current value at ``loc`` equals ``expected``. The original value in the
memory is returned.

``succOrd`` and ``failOrd`` must be at least ``MONOTONIC``.

For LLVM users: This is similar to the ``cmpxchg`` instruction. ``volatile`` is
also not supported.

Example::

    // assume %r is iref<int<64>>

    %orig    = LOAD <int<64>> %r                // int<64>
    %squared = MUL <int<64>> %orig %orig        // int<64>
    %old     = CMPXCHG ACQ_REL MONOTONIC
            <int<64>> %r %orig %squared         // int<64>

    %success = EQ <int<64>> %orig %old
    BRANCH2 %eq %cont %failed

Example 2::

    %entry:
        // assume %r is iref<int<64>>
        %orig    = LOAD <int<64>> %r                // int<64>

    %loop:
        %cmp     = PHI <int<64>> { %entry: %orig; %loop: %old; }
        %squared = MUL <int<64>> %cmp %cmp          // int<64>
        %old     = CMPXCHG ACQ_REL MONOTONIC
                    <int<64>> %r %cmp %squared      // int<64>
        %success = EQ <int<64>> %orig %old
        BRANCH2 %success %done %loop

    %done:
        // continue here

- ``ATOMICRMW atomicOrd atomicRMWOp < T > loc opnd``

    ``atomicOrd``
        *atomic ordering*: requirement for atomicity and memory ordering.
    ``atomicRMWOp``
        *AtomicRMW operator*: The operation to do.
    ``T``
        *type*: The type ``loc`` refers to.
    ``loc``
        *value* of ``iref<T>``: The reference to do operation.
    ``opnd``
        *value* of ``T``: The right-hand-side of the operation.  If ``T`` is
        ``weakref<U>``, then ``opnd`` has type ``ref<U>`` instead of
        ``weakref<U>``.
    return value
        Type ``T``: The original value in the memory.

+------+------------+-------------+-----+------+------+
| opct | opct       | opct        | idt | idt  | idt  |
+======+============+=============+=====+======+======+
| 0x1D | atomicOrd  | atomicRMWOp | T   | loc  | opnd |
+------+------------+-------------+-----+------+------+

An **AtomicRMW operator** is one of the following:

=========== ====== ======= =============
Keyword     Binary  T      Semantic
=========== ====== ======= =============
XCHG        0x00   int/ref exchange
ADD         0x01   int     add
SUB         0x02   int     subtract
AND         0x03   int     bitwise and
NAND        0x04   int     bitwise nand
OR          0x05   int     bitwise or
XOR         0x06   int     bitwise xor
MAX         0x07   int     signed max
MIN         0x08   int     signed min
UMAX        0x09   int     unsigned max
UMIN        0x0A   int     unsigned min
=========== ====== ======= =============

Load from ``loc``, perform ``atomicRMWOp`` with the loaded value and ``opnd``
and store the result back into the memory as one atomic operation. The original
value in the memory is returned.

All operations work on integer types and only ``XCHG`` also work on
``ref``, ``iref`` or ``weakref`` type.

For LLVM users: This is similar to the ``atomicrmw`` instruction. ``volatile``
is also not supported.

Example::

    // assume %r is iref<int<64>>
    %old = ATOMICRMW ACQ_REL ADD <int<64>> %r 42

Fence
-----

- ``FENCE atomicOrd``

    ``atomicOrd``
        *atomic ordering*: requirement for atomicity and memory ordering.

+------+------------+
| opct | opct       |
+======+============+
| 0x1E | atomicOrd  |
+------+------------+

This is an explicit fence which prevents specific machine instructions to be
reordered across this fence in the underlying processors.

For LLVM users: This is the counterpart of the ``fence`` instruction.

Trap Instructions
=================

.. _inst-trap:
.. _inst-watchpoint:

- ``TRAP < T > nor exc KEEPALIVE ( v1 v2 ... )``
- ``WATCHPOINT wpid < T > dis nor exc KEEPALIVE ( v1 v2 ... )``

    ``wpid``
        *intImmediate*: Watchpoint identifier.
    ``T``
        *type*: The return type.
    ``dis``
        *label*: The destination when the ``WATCHPOINT`` is not
        enabled.
    ``nor``, ``exc``
        *label*: The normal destination and the exceptional destination when
        the ``WATCHPOINT`` is enabled.
    ``v1``, ``v2``, ...
        *value*: Local SSA Values to keep alive.
    return value:
        Type ``T``: The value returned from the client if the client chooses to
        return normally.

``TRAP``:

+------+-----+-----+-----+------+-----+-----+-----+
| opct | idt | idt | idt | lent | idt | idt | ... |
+======+=====+=====+=====+======+=====+=====+=====+
| 0xE8 | T   | nor | exc | nka  | v1  | v2  | ... |
+------+-----+-----+-----+------+-----+-----+-----+

``WATCHPOINT``:

+------+------+-----+-----+-----+-----+------+-----+-----+-----+
| opct | idt  | idt | idt | idt | idt | lent | idt | idt | ... |
+======+======+=====+=====+=====+=====+======+=====+=====+=====+
| 0xE9 | wpid | T   | dis | nor | exc | nka  | v1  | v2  | ... |
+------+------+-----+-----+-----+-----+------+-----+-----+-----+

The ``TRAP`` instruction is an upcall to the client. It transfers the
control to a trap handler in the µVM Client. The client may do anything. It may
return normally, throw an exception, or does not return at all. When the client
chooses to return normally, the client may return a value back to this
instruction and that value must have type ``T``.

``v1``, ``v2``, ... are local SSA Values kept alive so that the client can
introspect the value of those Values.

The ``WATCHPOINT`` instruction is a conditional variance of trap. It is
not enabled in the beginning. The client can asynchronously (in another thread)
enable any watchpoint identified by an integer watchpoint ID. All
``WATCHPOINT`` instructions of that ID are enabled since then and behave
like a ``TRAP``. When the ``WATCHPOINT`` is not enabled, it
unconditionally branch to ``dis``.

In the binary form, the number of keepalive values is decided by ``nka``.

For LLVM users: LLVM has the ``llvm.trap`` intrinsic function, but its semantic
is not defined.

Example 1: use ``TRAP`` to handle the case when ``%someVal != 42``::

    %bb1:
        %eq = EQ <int<64>> %someVal 42
        BRANCH2 %eq %cont %trap

    %cont:
        // normal continue here

    %trap:
        TRAP <void> %nocont %nocont KEEPALIVE (%someVal)

    %nocont:
        THROW 0 // This should not be reached, but when it is,
                // it throws an exception whose value is null.

Example 2: use ``TRAP`` as a client-assisted native function::

    .funcdef @print <int<64> (iref<int<8>>)> (%str) {
    %entry:
        %bytes_printed = TRAP <int<64>> %cont %fail KEEPALIVE (%str)

    %cont:
        RET <int<64>> %bytes_printed

    %fail:
        %exc = LANDINGPAD
        THROW %exc
    }

C Foreign Function Interface
============================

- ``CCALL callconv < sig > func ( arg1 arg2 ... )``

    ``callconv``
        *calling convention*: The calling convention.
    ``sig``
        *function signature*: The signature of the callee.
    ``func``
        *value* of ``int<WORD_LENGTH>``: The callee's memory address.
    ``argx``
        *value* according to ``sig``: Parameter.
    return value
        Type decided by ``sig``: The return value of the callee.

+------+----------+-----+------+-------+------+------+-----+
| opct | opct     | idt | idt  | lent  | idt  | idt  | ... |
+======+==========+=====+======+=======+======+======+=====+
| 0xEC | callconv | sig | func | nargs | arg1 | arg2 | ... |
+------+----------+-----+------+-------+------+------+-----+

*calling convention* is one in the following table:

=========== ======
Keyword     Binary
=========== ======
DEFAULT     0x00
=========== ======

The ``CCALL`` instruction calls a native C function.

Currently only the ``DEFAULT`` calling convention is support, which is the
default calling convention according to the application binary interface (ABI)
of the current platform.

``func`` is the address of the C function. This address is obtained elsewhere,
either provided by the client which loads shared objects (.so), gets the address
of a particular C function before generating µVM IR code, or by calling the
``dlsym`` C function directly from µVM IR code using this ``CCALL``
instruction to get the address of other C functions.

The arguments and the return values are converted to be compatible with C.

Parameter types:

=========== =========== ========================================================
µVM type    C type      Note
=========== =========== ========================================================
int<len>    int types   the C type of the same length.
float       float
double      double
ref         pointer     pins heap object
iref        pointer     pins heap object
int<WORD>   pointer     When C expects pointer, it is possible to pass integer.
struct      struct
void        void
(others)    (forbidden)
=========== =========== ========================================================

Memory types:

=========== =========== ========================================================
µVM type    C type      Note
=========== =========== ========================================================
int<len>    int types   the C type of the same length.
float       float
double      double
int<WORD>   pointer     C pointers are treated as integers by the µVM.
struct      struct      use the default layout.
void        void
(others)    (forbidden)
=========== =========== ========================================================

If any argument has type ``ref`` or it is ``iref`` and points into a heap
object, then that object is pinned during this call. If multiple threads are
making ``CCALL`` using the same object as parameters, then as long as
there is one thread still in the process of a ``CCALL``, the object will
remain to be pinned.

Since not all C types have a µVM counterpart, µVM is only able to call the C
functions with the simplest signatures. This should suffice for most system
calls, which is the primary target of this instruction. Sometimes writing a
simple wrapper function in C is needed.

``CCALL`` cannot receive exceptions thrown by C++.

``CCALL`` is not an OSR point.

For LLVM user: µVM is not designed to be compatible with C and functions defined
in µVM IR does not use the native C ABI. This instruction is necessary to
communicate with other parts of the system, especially operating systems since
most operating systems are currently written in C and provide C interfaces.

Example::

    // The client loads libc and gets the address for printf
    .const @printf_address <int<64>> = 0x0011223344556600   // hard-code
    
    %h = ALLOCAHYBRID <hybrid<void, int<8>>> 100
    %v = GETVARPARTIREF <hybrid<void, int<8>>> %h
    // fill %v with string "Hello world\n\0"
    CCALL DEFAULT <int<32> (iref<int<8>>)> @printf_address (%v)

Thread and Stack
================

.. _inst-newstack:

- ``NEWSTACK < sig > func ( arg1 arg2 ... )``

    ``sig``
        *function signature*: The signature of the callee.
    ``func``
        *value* of ``int<WORD_LENGTH>``: The callee's memory address.
    ``argx``
        *value* according to ``sig``: Parameter.

+------+-----+------+-------+------+------+-----+
| opct | idt | idt  | lent  | idt  | idt  | ... |
+======+=====+======+=======+======+======+=====+
| 0xE0 | sig | func | nargs | arg1 | arg2 | ... |
+------+-----+------+-------+------+------+-----+

The ``NEWSTACK`` instruction creates a new stack with an about-to-begin
function activation at the bottom.

The new stack is in the **READY** state.

See `<intrinsic-funcs>`__ for more operations for stacks and threads.

Intrinsic Function
==================

Intrinsic functions are a mechanism so that the µVM IR can be extended without
adding new instructions or changing the grammar.

In µVM, intrinsic functions work just like regular instructions, but has a
unified form: they take only value parameters, may have a normal destination
and an exceptional destination and may keep some local SSA Value alive.
Theoretically, any instructions that matches these criteria can be implemented
as intrinsic functions. For example: throw and landingpad. In practice, they are
so special that they deserve their own instruction.

- ``ICALL func ( arg1 arg2 ... )``
- ``ICALL func ( arg1 arg2 ... ) KEEPALIVE ( v1 v2 ... )``
- ``IINVOKE func ( arg1 arg2 ... ) nor exc``
- ``IINVOKE func ( arg1 arg2 ... ) nor exc KEEPALIVE ( v1 v2 ... )``

    ``func``
        *global identifier*: identifier for the intrinsic function.
    ``arg1``, ``arg2``, ...
        *value*: arguments to the intrinsic function
    ``nor``, ``exc``
        *label*: the normal and the exceptional destination, respectively.
    ``v1``, ``v2``, ...
        *value*: other SSA Values to be kept alive during this "call".
    return value:
        Type determined by the respective intrinsic functions. See
        `<intrinsic-funcs>`__.

ICALL:

+------+------+-------+------+------+-----+------+-----+-----+-----+
| opct | idt  | lent  | idt  | idt  | ... | lent | idt | idt | ... |
+======+======+=======+======+======+=====+======+=====+=====+=====+
| 0xE4 | func | nargs | arg1 | arg2 | ... | nka  | v1  | v2  | ... |
+------+------+-------+------+------+-----+------+-----+-----+-----+

IINVOKE:

+------+------+-----+-----+-------+------+------+-----+------+-----+-----+-----+
| opct | idt  | idt | idt | lent  | idt  | idt  | ... | lent | idt | idt | ... |
+======+======+=====+=====+=======+======+======+=====+======+=====+=====+=====+
| 0xE5 | func | nor | exc | nargs | arg1 | arg2 | ... | nka  | v1  | v2  | ... |
+------+------+-----+-----+-------+------+------+-----+------+-----+-----+-----+

``ICALL`` and ``IINVOKE`` calls a intrinsic function.

The parser has knowledge of all intrinsic functions, including their expected
parameter types, their return type, whether exceptions may be thrown and whether
it is an OSR point. ``IINVOKE`` only makes sense for potential excepting
instructions. The ``KEEPALIVE`` clause only makes sense for OSR point functions.
Unlike regular function calls, not all intrinsic functions are OSR points.

Each intrinsic function has an identifier which is global (starting with ``@``)
and its name starts with a ``uvm.`` prefix. It also has a numerical identifier
for the binary form. Both are documented.

See `<intrinsic-funcs>`__ for a list of intrinsic functions.

For LLVM users: µVM treats intrinsic functions differently from regular
functions and has distinct instructions. LLVM reuses the ``call`` and the
``invoke`` instructions.

Example::

    // math functions
    %v1 = ICALL @uvm.sin (1.23)
    %v2 = ICALL @uvm.cos (1.23)
    %v3 = ICALL @uvm.tan (1.23)
    %v4 = ICALL @uvm.sqrt (2.0)

    // extra stack/thread operations
    %s = NEWSTACK <void ()> @foo ()
    %t = ICALL @uvm.new_thread (%s)

.. vim: tw=80
