clutch
======

`clutch` is a simple type erasure library. Basically it supports polymorphism through type-erasure and not by virtual
functions. Therefore classes not need to be inherited from some interface.

Another advantage that erased types can have value semantics, so no need to wrap them into some kind of pointer.

On the top of that, dynamic memory usage can be avoided (and small buffer optimization will be supported as well).
