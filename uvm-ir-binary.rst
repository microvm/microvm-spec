==================
µVM IR Binary Form
==================

This document describes the binary form of the µVM intermediate representation.
For the text form, see `<uvm-ir>`__.

Overview
========

The µVM IR Binary Form is similar to the `Text Form <uvm-ir>`__ in structure,
but has notable differences.

Numerical ID
------------

The Binary Form identifies objects using numerical identifiers rather than
textual identifiers.  In this document, **ID** means the numerical identifier
while **name** means the textual identifier.

An **ID** is a 32-bit unsigned integer. It is used so that one defined object
can refer to others. Types, function signatures, constants, global data,
functions, parameters, instructions and intrinsic functions have IDs.

An **ID** uniquely identifies one object of the kinds above. There is no
distinction between "global" ID and "local" ID as there is in the text form.
However, an ID may refer to related but different objects in different contexts.
For example, the ID of a constant may refer to the constant definition itself,
but also the SSA Value which is a constant of the type defined in the
definition. The ID of a function may refer to the function itself and can be
redefined using the ID, but also an SSA Value which is a constant of the
``func`` type, referring to that function.

Zero (0) is an invalid ID. ID numbers within 1-65535 are reserved by the µVM.
Other numbers are free for the client to use.

No nested or implicit definitions
---------------------------------

Unlike the text form, every object, including types, signatures and constants,
must be defined separately, not nested. For example, the following is valid in
the text form::

    .typedef @Foo = struct <float struct <int<64> double>>  // nested struct

    .const @foo <@Foo> = {100.0f, {42, 3.14d}}              // nested constant

    .funcdef @add1 <int<32> (int<32>)> (%a) { // signature (including its int<32>) must be defined separately
        %b = ADD <int<32>> %a 1     // int<32> and 1 should be defined separately.
    }

However, in the binary form, every declaration must contain exactly one
constructor (type constructor or constant literal) and nested constructors must
be moved to their own definitions. The example above must be written in binary
equivalent to this form::

    .typedef @i64  = int<64>
    .typedef @d    = double
    .typedef @f    = float
    .typedef @Foo1 = struct <@i64 @d>
    .typedef @Foo  = struct <@f @Foo1>

    .const @forty_two <@i64>  = 42
    .const @pi        <@d>    = 3.14d
    .const @foo1      <@Foo1> = {@forty_two, @pi}
    .const @fpnum100  <@f>    = 100.0f
    .const @foo       <@Foo>  = {@fpnum100, @foo1}

    .typedef @i32 = int<32>
    .funcsig @add1_sig = @i32 (@i32)
    .const @one <@i32> = 1
    .funcdef @add1 <@add1_sig> (%a) {
        %b = ADD <@i32> %a @one
    }

NOTE: The reason for this rule is consistency. By doing this, every definition
creates exactly one object. All object refers to others using IDs rather than
containing them. For most programming languages, there are only a few primitive
types which can be exhaustively defined before writing other code. For
user-defined types (classes, structs, etc.), they have to be defined and the
number of types is far smaller than the number of uses. So it is worth defining
them separately.

Take Java as an example. It is recommended to define all primitive types before
other code::

    .typedef @byte      = int<8>
    .typedef @short     = int<16>
    .typedef @int       = int<32>
    .typedef @long      = int<64>
    .typedef @char      = int<16>
    .typedef @boolean   = int<32>
    .typedef @float     = float
    .typedef @double    = double
    .typedef @void      = void
    .typedef @Object    = struct <...>
    .typedef @ObjectRef = ref<@Object>

    .typedef @bytearray    = hybrid<@int @byte>    // @int is for the length
    .typedef @shortarray   = hybrid<@int @short>
    .typedef @intarray     = hybrid<@int @int>
    .typedef @longarray    = hybrid<@int @long>
    .typedef @chararray    = hybrid<@int @char>
    .typedef @booleanarray = hybrid<@int @byte>    // different
    .typedef @floatarray   = hybrid<@int @float>
    .typedef @doublearray  = hybrid<@int @double>
    .typedef @refarray     = hybrid<@int @ObjectRef>

When loading a class, the client should define a struct type itself, a ref to it
and an iref to it::

    .typedef @String     = struct <...>
    .typedef @StringRef  = ref<@String>
    .typedef @StringIRef = iref<@String>

Binary format
=============

A bundle in the binary form consists of many numbers encoded in bytes. All
numbers are encoded in **little endian** and are **tightly packed** which means
there are no padding bytes between two adjacent numbers for alignment. For
floating point numbers, it is equivalent to convert them bit-by-bit into integer
types of the same length and convert to bytes in little endian.

This document uses the following notation for types:

i8, i16, i32, i64
    Integer types of the respective lengths.
float, double
    Floating point types of 32 bits and 64 bits, respectively.
idt
    The type of binary IDs. Alias to i32.
lent
    The type of lengths, used for the number of items contained in
    variable-length definitions (e.g. struct type and parameter list). Alias to
    i16.
aryszt
    The type of array sizes, used for the number of items contained in the array
    type. Alias to i64.
opct
    The opcode type, but not just used for instruction opcodes. Alias to i8.

For each concrete construction, the binary format appears in a table. It has two
rows. The first row is a list of binary types of the respective column and the
second row specifies for each column either a symbolic name for that field or
the exact binary content expected.

+-------+------------------+
| type1 | type2            |
+=======+==================+
| num   | or symbolic name | 
+-------+------------------+

The general binary format follows the type-length-value principle. Most
definitions can identify its length by its opcode. Other variable-length
definitions are preceded by a field of ``lent`` indicating the number of
elements.

Top-level Structure
===================

In the binary form, the top-level definitions are type definitions, function
signature definitions, constant definitions, global data definitions, function
definitions, function declarations and name bindings.

Only name bindings are not in the text form. It is for associating numerical IDs
to names for debugging purposes.

Type Definition
---------------

Type definition has the following form:

+------+-----+------------------+
| opct | idt |                  |
+======+=====+==================+
| 0x01 | id  | type constructor |
+------+-----+------------------+

``id`` is the identifier of the defined type. A type constructor follows the
opcode 0x01 and the id. See `<type-system>`__ for a complete list of type
constructors.

NOTE: this is equivalent to: ``.typedef id = type constructor``.

Function Signature Definition
-----------------------------

Function signature definition has the following form:

+------+-----+-------+---------+----------+----------+-----+
| opct | idt | idt   | lent    | idt      | idt      | ... |
+======+=====+=======+=========+==========+==========+=====+
| 0x02 | id  | retty | nparams | paramty1 | paramty2 | ... |
+------+-----+-------+---------+----------+----------+-----+

``id`` is the identifier of the defined function signature. ``retty`` is the
ID of the return type. ``nparams`` is the number of parameters and it is
followed by nparams IDs each specifying the type of each parameter.

NOTE: this is equivalent to: ``.funcsig id = retty (paramty1 paramty2 ...)``

Constant Definition
-------------------

Constant definition has the following form:

+------+-----+------+----------------------+
| opct | idt | idt  |                      |
+======+=====+======+======================+
| 0x03 | id  | type | constant constructor |
+------+-----+------+----------------------+

``id`` is the identifier of the defined constant. ``type`` is the type of the
constant and must match the constant constructor. A constant constructor follows
the type.

NOTE: this is equivalent to: ``.const id <type> = constant constructor``

Integer Constant Constructor
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

An integer constant constructor has the following form:

+------+--------+
| opct | i64    |
+======+========+
| 0x01 | number |
+------+--------+

``number`` is the integer constant number. Although all integer constant
constructors have 64-bit numbers, only the lowest ``n`` bits are valid if the
type specified in the Constant Definition is ``int<n>``. Extra high bits are
removed.

NOTE: this is equivalent to an integer literal in the text form.

Floating Point Constant Constructors
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

A float constant constructor has the following form:

+------+--------+
| opct | float  |
+======+========+
| 0x02 | number |
+------+--------+

``number`` is the float constant number.

NOTE: this is equivalent to a float literal in the text form.

A double constant constructor has the following form:

+------+--------+
| opct | double |
+======+========+
| 0x03 | number |
+------+--------+

``number`` is the double constant number.

NOTE: this is equivalent to a double literal in the text form.

Struct Constant Constructor
~~~~~~~~~~~~~~~~~~~~~~~~~~~

A struct constant constructor has the following form:

+------+---------+--------+--------+-----+
| opct | lent    | idt    | idt    | ... |
+======+=========+========+========+=====+
| 0x04 | nfields | field1 | field2 | ... |
+------+---------+--------+--------+-----+

``nfields`` is the number of fields. It is followed by nfields identifiers, each
of which refers to another constant which will be the field of the current
struct constant.

NOTE: this is equivalent to struct literal: ``{field1 field2 ...}``

NULL Constant Constructor
~~~~~~~~~~~~~~~~~~~~~~~~~

A NULL constant constructor has the following form:

+------+
| opct |
+======+
| 0x05 |
+------+

Only an opcode.

NOTE: this is equivalent to the ``NULL`` keyword in the text form.

Global Data Definition
----------------------

Global data definition has the following form:

+------+-----+------+
| opct | idt | idt  |
+======+=====+======+
| 0x04 | id  | type |
+------+-----+------+

``id`` is the ID of the defined global data. ``type`` is the type of the global
data.

NOTE: this is equivalent to: ``.global id <type>``

Function Definition and Declaration
-----------------------------------

Function declaration is shorter and has the following form:

+------+-----+-----+
| opct | idt | idt |
+======+=====+=====+
| 0x05 | id  | sig |
+------+-----+-----+

``id`` is the ID of the declared function. ``sig`` is the function signature of
it.

NOTE: this is equivalent to: ``.funcdecl id <sig>``

Function definition is longer and has the following form:

+------+-----+-----+---------+--------+--------+-----+------+---------------+
| opct | idt | idt | lent    | idt    | idt    |     | i32  |               |
+======+=====+=====+=========+========+========+=====+======+===============+
| 0x06 | id  | sig | nparams | param1 | param2 | ... | nbbs | function body |
+------+-----+-----+---------+--------+--------+-----+------+---------------+

``id`` is the ID of the defined function. ``sig`` is the function signature of
it. ``paramx`` are identifiers of parameters. ``nbbs`` is the number of basic
blocks in the function body.

NOTE: this is equivalent to: ``.funcdef id <sig> (param1 param2 ...) { function
body }``

Function Body
=============

Inside the function body, there are many basic blocks and each basic block has
many instructions.

A basic block has the following form:

+-----+--------+-----------------+
| idt | i32    |                 |
+=====+========+=================+
| id  | ninsts | instructions... |
+-----+--------+-----------------+

``id`` is the ID of the basic block. Every basic block must have an ID,
*even the entry block*. ``ninsts`` is the number of instructions in the current
basic block. There are ninsts basic blocks following the header.

An instruction has the following form:

+-----+------------------+
| idt |                  |
+=====+==================+
| id  | instruction body |
+-----+------------------+

``id`` is the ID of the instruction. ``instruction body`` is specific to each
instruction. See `<instruction-set>`__ for an exhaustive list.

Name Binding
============

Name binding is a definition specific to the binary form. It binds a name to an
ID. It is designed for debugging purposes and is optional. The name must be a
valid textual identifier (prefix with '@' for global and '%' for local).

A name binding has the following form:

+------+-----+--------+-------+-------+-----+
| opct | idt | lent   | i8    | i8    | ... |
+======+=====+========+=======+=======+=====+
| 0x07 | id  | nbytes | byte1 | byte2 | ... |
+------+-----+--------+-------+-------+-----+

``id`` is the ID to bind. ``nbytes`` is the number of bytes in the name and
``bytex`` is the value of each byte.

The name is encoded in UTF-8.

Example
=======

Here is a side-by-side translation of a bundle from the text form to the binary
form::

    //       @i8
    .typedef @65536     = int <8>
    // 01    00 00 01 00  01  08

    //       @i64
    .typedef @65537     = int <64>
    // 01    01 00 01 00  01  40
    
    //       @float
    .typedef @65538     = float
    // 01    02 00 01 00  02

    //       @double
    .typedef @65539     = double
    // 01    03 00 01 00  03

    //       @refi8              @i8
    .typedef @65540     = ref   <@65536>
    // 01    04 00 01 00  04    00 00 01 00

    //       @irefi64            @i64
    .typedef @65541     = iref  <@65537>
    // 01    05 00 01 00  05    01 00 01 00

    //       @wreff                  @float
    .typedef @65542     = weakref   <@65538>
    // 01    06 00 01 00  06        02 00 01 00

    //       @s                     2 flds  @i8             @i64
    .typedef @65543     = struct    <       @65536          @65537>
    // 01    07 00 01 00  07        02 00   00 00 01 00     01 00 01 00

    //       @a                      @i8
    .typedef @65544     = array     <@65536         10>
    // 01    08 00 01 00  08        00 00 01 00     0a 00 00 00 00 00 00 00

    //       @h                      @i64           @i8
    .typedef @65545     = hybrid    <@65537         @65536>
    // 01    09 00 01 00  09        00 00 01 00     01 00 01 00

    //       @void
    .typedef @65546     = void
    // 01    0a 00 01 00  0a

    //       @f                  @sig2
    .typedef @65547     = func  <@65552>
    // 01    0b 00 01 00  0b    10 00 01 00

    //       @thread
    .typedef @65548     = thread
    // 01    0c 00 01 00  0c

    //       @stack
    .typedef @65549     = stack
    // 01    0d 00 01 00  0d

    //       @tagref64
    .typedef @65550     = tagref64
    // 01    0e 00 01 00  0e

    //       @sig1        @void
    .funcsig @65551     = @65546        ()      // void ()
    // 02    0f 00 01 00  0a 00 01 00   00 00

    //       @sig2        @i64                 @i64         @irefi64
    .funcsig @65552     = @65537        (      @65537       @65541)
    // 02    10 00 01 00  0a 00 01 00   02 00  01 00 01 00  05 00 01 00

    //      @const0       @i8
    .const  @65568       <@65536>    =  42
    // 03   20 00 01 00   00 00 01 00   01  2a 00 00 00 00 00 00 00

    //      @const1       @i64
    .const  @65569       <@65537>    =  0x123456789abcdef0
    // 03   21 00 01 00   01 00 01 00   01  f0 de bc 9a 78 56 34 12

    //      @const2       @float
    .const  @65570       <@65538>    =  1234.5f
    // 03   22 00 01 00   02 00 01 00   02  00 50 9a 44

    //      @const3       @double
    .const  @65571       <@65539>    =  1234.5d
    // 03   23 00 01 00   03 00 01 00   03  00 00 00 00 00 4a 93 40

    //      @const4       @s                2 flds   @const0      @const1
    .const  @65572       <@65543>    =  {            @65568       @65569 }
    // 03   24 00 01 00   07 00 01 00   04  02 00    20 00 01 00  21 00 01 00

    //      @const5       @refi8
    .const  @65573       <@65540>    =  NULL
    // 03   25 00 01 00   04 00 01 00   05

    //      @gd0          @refi8
    .global @65584       <@65540>
    // 04   30 00 01 00   04 00 01 00

    //          @func0       @sig1
    .funcdecl   @65600      <@65551>
    // 05       40 00 01 00  0f 00 01 00

    //          @func1       @sig2          2 params    %p0          %p1          1 basic block
    .funcdef    @65601      <@65552>        (           %65602       %65603)      {
    // 06       41 00 01 00  10 00 01 00    02 00       42 00 01 00  43 00 01 00  01 00 00 00

    //  %entry       3 instructions
        %65604:
    //  44 00 01 00  03 00 00 00

            // %v1                           @i64         %p1
            %65605 =        LOAD NOT_ATOMIC <@65537>      %65603
            // 45 00 01 00  1a   00          01 00 01 00  43 00 01 00

            // %v2                @i64          %p0          %v1
            %65606 =        ADD  <@65537>       %65602       %65605
            // 46 00 01 00  01   01 00 01 00    42 00 01 00  45 00 01 00

            %65607 =        RET  <@65537>       %v2
            // 47 00 01 00  63   01 00 01 00    46 00 01 00
        
    }

    // Bind ID 65604 with the string "%entry"
    07  44 00 01 00  06  25 65 6e 74 72 79

.. vim: textwidth=80
