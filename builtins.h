#ifndef BUILTINS_H
#define BUILTINS_H

#define builtin_binop_list \
  X(add, +)  \
  X(sub, -)  \
  X(mult, *) \
  X(div, /)

#define builtin_list \
  X(car, CAR)   \
  X(cdr, CDR)   \
  X(cons, CONS) \
  builtin_binop_list

#define X(name, op) int builtin_##name (Atom args, Atom *result);
builtin_list
#undef X

#endif
