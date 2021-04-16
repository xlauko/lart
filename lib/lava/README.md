# LAVA: A Library of Abstract VAlues

This is a header-only C++ library which provides a number of abstract (value)
domains. Each domain is implemented as a class (or class template, for
parametric domains) and provides a number of methods which implement the
individual operations of the domain. Not all domains need to implement all
operations (more on that below), but all operations come from a fixed set that
LART and LAMP understand.

The domains in LAVA can come in three basic flavours, depending on the
operations and semantics they possess.

## Scalar Domains

These represent values that behave as the familiar scalars of a programming
language: integers, floating-point numbers, booleans and similar types, each
made up of a fixed number of bits with specific semantics. A scalar domain
needs to implement basic operations from the LLVM instruction set: arithmetic,
bit manipulation, conversions and comparisons (see `lamp/wrapper.hpp` for a
definitive list).

## Array Domains

An array domain has two other associated domains: the scalar 'index' domain,
and the 'value' domain (which can be of arbitrary type and not necessarily a
scalar). Arrays, unlike scalars, also have an 'identity' (an abstraction of
'address') and are *mutable*: the content of an array can be changed during a
computation, but the altered array will still have the same identity.

Since it is advantageous to model program memory as a disjoint set of objects
(instead of a flat array of bytes), the indices we use are also split into two
parts: one that identifies the object and another, which corresponds to the
byte offset into that object. This essentially means that the 'index' domain
models tuples (object, offset).
