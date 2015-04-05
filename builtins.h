#ifndef BUILTINS_H
#define BUILTINS_H

#define builtin_binop_list \
  X(add, +)  \
  X(sub, -)  \
  X(mult, *) \
  X(div, /)

#define builtin_list \
  X(car, CAR)     \
  X(cdr, CDR)     \
  X(cons, CONS)   \
  builtin_binop_list

int builtin_car (Atom args, Atom *result);
int builtin_cdr (Atom args, Atom *result);
int builtin_cons (Atom args, Atom *result);

#define X(name, op) int builtin_##name (Atom args, Atom *result);
builtin_binop_list
#undef X

#endif
