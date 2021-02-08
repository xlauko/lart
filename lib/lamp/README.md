# LAMP: A Library of Abstract Metadomain Packages

Each C++ source file (module) in this directory implements a single abstract
meta-domain, which mediates interaction between abstract domains from LAVA
(which represent values at runtime), and the abstract program as generated
(transformed) by LART.

A program abstracted by LART, before it is executed or further analysed in
some way, needs to be linked to exactly one of the LAME modules (in bitcode
form). Interface-wise, all modules are equivalent and LART does not need to
know which is going to be used. The difference lies in which abstract domains
they combine, and how -- i.e. different LAME modules will give different
abstract semantics to the same program (syntactically) abstracted by LART.

The C interface between LART and LAMP is defined in `wrapper.hpp` (the same
file also effectively describes the C++ interface between LAMP and LAVA, via
the C++ calls the C wrappers perform).
