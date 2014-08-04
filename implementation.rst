==============
Implementation
==============

There are two aspects of the MicroVM, namely the specification and the
implementation.  

Some details of the MicroVM are not specified by the specification and are left
to the implementation. They include:

- The underlying platform, i.e., the hardware, the word length (16-bit, 32-bit,
  64-bit, ...), the endianness (big-endian, little-endian, ...), the operating
  system, etc.
- The programming language and the runtime environment the µVM itself is
  implemented in and run on. It is not always practical to run the µVM upon
  another VM, but the specification does not force implementing µVM natively.
- The way µVM interfaces with the client. They can be the same program, in the
  same process or in different processes. They can be written in the same
  language or different languages.
- The way µVM IR code is represented and transmitted. The µVM specification
  defines the µVM IR, including the type system, the instruction set, etc.,
  their semantics and two representations (text and binary) for interchanging.
  However, for practical reasons, the client may represent the µVM IR in
  equivalent formats.
- The way µVM IR code is executed. The µVM is designed for the convenience of
  just-in-time compiling, but it does not forbid implementing as an interpreter.
- The way µVM threads are implemented. The behaviour of µVM threads are
  specified and the µVM is designed with simultaneous multi-threading in mind,
  but the implementation may or may not implement threading using native
  threads.
- The concrete garbage-collection algorithm used. The semantic of references is
  defined and the implementation can choose any garbage collector as deems
  appropriate.

Implementations should advertise their specific details to their users.


.. vim: tw=80
