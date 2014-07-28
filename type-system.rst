===========
Type System
===========

Notation
========


In this document, the text format appears as:

``sometype < type_param1 type_param2 ... >``

    ``type_param1``
        description for type param1
    ``type_param2``
        description for type param2

Type Constructors
=================

All µVM types are (potentially recursive) combinations of the following *type
constructors*.  By convention, types are written in lower cases. Type parameters
are written in angular brackets ``< >``.

The binary form of the type constructors are shown in the table after each type
constructor.

- int < length >
- float
- double
- ref < T >
- iref < T >
- weakref < T >
- struct < T1 T2 ... >
- array < T length >
- hybrid < F V >
- void
- func < sig >
- thread
- stack
- tagref64

Numeric Types
=============

``int < length >``

    ``length``
        *intLiteral*: The length of the integer in bits.

+------+--------+
| opct | i8     |
+======+========+
| 0x01 | length |
+------+--------+

``float``

+------+
| opct |
+======+
| 0x02 |
+------+

``double``

+------+
| opct |
+======+
| 0x03 |
+------+

``int`` is the only integer type in µVM. *length* is the length of the
integer in bits. Boolean values accepted by the ``SELECT`` and
``BRANCH2`` instructions are represented as ``int<1>``, but the client
decides what µVM type the high-level language boolean type maps to.

Integer values can be interpreted as either signed or unsigned. The signedness
is determined by the instructions rather than the type. For example, for the
division operation, the ``UDIV`` instruction treats its both operands as
unsigned integers and ``SDIV`` treats them as signed integers. When an integer
is treated as signed, it uses the 2's complement representation where the
highest bit is the sign bit.

``float`` and ``double`` are single-precision and
double-precision floating point numbers, respectively.

For LLVM users: these types are directly borrowed from LLVM.

Example::

    int<1>
    int<8>
    int<16>
    int<32>
    int<64>
    float
    double

Reference Types
===============

``ref < T >``

    ``T``
        *type*: The type of referent.

+------+-----+
| opct | idt |
+======+=====+
| 0x04 | T   |
+------+-----+

``iref < T >``

    ``T``
        *type*: The type of referent.

+------+-----+
| opct | idt |
+======+=====+
| 0x05 | T   |
+------+-----+

``weakref < T >``

    ``T``
        *type*: The type of referent.

+------+-----+
| opct | idt |
+======+=====+
| 0x06 | T   |
+------+-----+

``ref`` is an object reference. It always refers to an object on the
heap; ``iref`` is an internal reference: it refers to a field inside an
object on the heap or on the stack. The *T* parameter is always the type this
reference refers to.

``ref`` and ``iref`` may have value null, which is represented
by literal 0 in the text form. It is the only allowed invalid reference. All
null references are equal. Deriving an ``iref`` from a null
``ref`` is meaningless.

``weakref`` is a weak object reference. Its referent will be garbage
collected if there is no strong references (``ref`` or ``iref``)
to it. In the event that the space of the referent is reclaimed and the
``weakref`` is no longer valid, its content will become null (0).

There is no weak internal reference.

``weakref`` is not SSA Value. It must be in the memory (heap or stack).
``LOAD`` from a ``weakref`` yields a strong reference and a
strong reference can be ``STORE`` -ed into a ``weakref`` field.

µVM provides the client a mechanism to retain any ``weakref`` when its
referent is about to be collected. This is done in an implementation-specific
way and is beyond the scope of this specification.

For LLVM users: there is no equivalence in LLVM. µVM guarantees that all
references are identified both in the heap and in the stack and are subject to
garbage collection. The closest counterpart in LLVM is the pointer type, but µVM
does not encourage the use of pointers, though pointer types will be introduced
in µVM in the future.

Example::

    ref<int<32>>
    ref<struct<int<32> int<16> int<8> double float>>
    ref<array<int<8> 100>>
    iref<int<32>>
    iref<struct<int<32> int<16> int<8> double float>>
    iref<array<int<8> 100>>
    weakref<int<32>>
    weakref<struct<int<32> int<16> int<8> double float>>
    weakref<array<int<8> 100>>

Composite Types
===============

Struct
------

``struct < T1 T2 ... >``

    ``T1``, ``T2``, ``...``
        *type*: The type of fields.

+------+---------+-----+-----+-----+
| opct | lent    | idt | idt | ... |
+======+=========+=====+=====+=====+
| 0x07 | nfields | T1  | T2  | ... |
+------+---------+-----+-----+-----+

A struct is a Cartesian product type of several types. *T1*, *T2*, etc. are its
fields.

A struct cannot be an SSA Value if it has an array as its component or the
component of nested structs.

In the binary form, an integer literal ``nfields`` determines the number of
fields.  Exactly that number of type IDs follows the ``nfields`` literal.

For LLVM users: this is almost identical to LLVM's struct type, except it does
not allow embedded arrays.

Example::

    struct<>
    struct<int<32> int<16> int<8> double float>
    struct<struct<int<32> int<32>> float struct<int<8> double>>

Array
-----

``array < T length >``

    ``T``
        *type*: The type of elements.
    ``length``
        *intLiteral*: The number of elements.

+------+-----+--------+
| opct | idt | aryszt |
+======+=====+========+
| 0x08 | T   | length |
+------+-----+--------+

An array is a sequence of homogeneous data structure in the memory. *T* is the
type of its elements and *length* is the length of the array.

For LLVM users: **An array is always fixed-length**. There is no type for "array
of run-time-determined length" in the µVM type system. The closest counterpart
is the ``hybrid`` type.

Example::

    array<int<8> 4096>                  // array of 4096 bytes
    array<double 100>                   // array of 100 doubles
    array<struct<int<64> ref<void>> 16> // array of 16 long-ref pairs
    array<array<int<64> 1024> 1024>     // array of arrays

Hybrid
------

``hybrid < F V >``

    ``F``
        *type*: The type of the fixed part
    ``V``
        *type*: The type of the elements of the variable part

+------+-----+-----+
| opct | idt | idt |
+======+=====+=====+
| 0x09 | F   | V   |
+------+-----+-----+

A hybrid is a combination of a fixed-size prefix and a array-like
variable-length suffix whose length is decided at allocation time. *F* is the
type of the fixed-size prefix. *V* is the type of the **elements** of the
variable-length suffix.

Example::

    hybrid<int<64> int<8>>      // one int64 followed by many int8
    hybrid<
        struct<int<64> int<64> int<64>>     // three initial int64 headers
        double                              // followed by many doubles
        >
    hybrid<void int<8>>         // no header. Just many int8.

Void Type
=========

``void``

+------+
| opct |
+======+
| 0x0A |
+------+

The ``void`` type has no value. It is useful for functions that does not
return value, references that refer to undetermined type and the
``hybrid`` type that misses the fixed part.

Function Types
==============

``func < sig >``

    ``sig``
        *function signature*: The signature of the referred function.

+------+-----+
| opct | idt |
+======+=====+
| 0x0B | sig |
+------+-----+

``func`` is a type for function identifiers. It is an opaque identifier
of a µVM function. the signature *sig* determines the parameter types and return
type of the function.

µVM allows a function to be re-defined at run time. The ID of the re-defined
function will not change and all ``func`` values will automatically
refer to the newly defined function.

For LLVM users: the ``func`` type in µVM is not a pointer (though may be
implemented as a pointer underneath, but this cannot be depended on). It is
opaque and is not supposed to be introspected.

Example::

    func<int<64> (int<64> int<64>)>
    func<int<32> (int<32> iref<int<8>>)>
    func<void ()>

Opaque Types
============

Some types identify internal µVM data structures. The actual binary
representation of the values are not visible to the client.

``thread``

+------+
| opct |
+======+
| 0x0C |
+------+

``stack``

+------+
| opct |
+======+
| 0x0D |
+------+

``thread`` and ``stack`` represent µVM threads and µVM stacks,
respectively.  Only some special instructions (e.g. ``NEWSTACK``) or
intrinsic functions can work on them.

``tagref64``

+------+
| opct |
+======+
| 0x0E |
+------+

``tagref64`` is a union type of ``double``, ``int<52>`` and
``ref``. It occupies 64 bits. The type of the content can be tested at
run time using the ``@uvm.tr64_is_xxx`` family of intrinsic functions. Intrinsic
functions like ``@uvm.tr64_to_xxx`` and ``@uvm.tr64_from_xxx`` are for
converting them to and from regular primitive types.

When a ``tagref64`` contains an object reference, it can hold an ``int<6>`` together
as a user-defined tag. It is useful to store type information.

This type is only available on some architectures including x86-64 with 48-bit addresses.

.. vim: tw=80
