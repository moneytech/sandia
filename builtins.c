#include "basics.h"
#include "builtins.h"

#define X(name, op) int builtin_##name (Atom args, Atom *result) { \
    Atom a, b; \
    if (nilp(args) || nilp(cdr(args)) || !nilp(cdr(cdr(args)))) { \
        return Error_args; \
    } \
    a = car(args); \
    b = car(cdr(args)); \
    if (a.type != Atype_int || b.type != Atype_int) { \
        return Error_type; \
    } \
    *result = make_int(a.value.integer op b.value.integer); \
    return Error_ok; \
}

builtin_binop_list

#undef X

int builtin_car (Atom args, Atom *result) {
    if (nilp(args) || !nilp(cdr(args))) {
        return Error_args;
    }

    if (nilp(car(args))) {
        *result = nil;
    }
    else if (car(args).type != Atype_pair) {
        return Error_type;
    }
    else {
        *result = car(car(args));
    }

    return Error_ok;
}

int builtin_cdr (Atom args, Atom *result) {
    if (nilp(args) || !nilp(cdr(args))) {
        return Error_args;
    }

    if (nilp(car(args))) {
        *result = nil;
    }
    else if (car(args).type != Atype_pair) {
        return Error_type;
    }
    else {
        *result = cdr(car(args));
    }

    return Error_ok;
}

int builtin_cons (Atom args, Atom *result) {
    if (nilp(args) || nilp(cdr(args)) || !nilp(cdr(cdr(args)))) {
        return Error_args;
    }

    *result = cons(car(args), car(cdr(args)));

    return Error_ok;
}
