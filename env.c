#include "env.h"

Atom env_create (Atom parent) {
    return cons(parent, nil);
}

int env_get (Atom env, Atom symbol, Atom *result) {
    Atom parent = car(env);
    Atom bs = cdr(env);

    while (!nilp(bs)) {
        Atom b = car(bs);
        if (car(b).value.symbol == symbol.value.symbol) {
            *result = cdr(b);
            return Error_ok;
        }
        bs = cdr(bs);
    }

    if (nilp(parent)) {
        return Error_unbound;
    }

    return env_get(parent, symbol, result);
}

int env_set (Atom env, Atom symbol, Atom value) {
    Atom bs = cdr(env);
    Atom b = nil;

    while (!nilp(bs)) {
        b = car(bs);
        if (car(b).value.symbol == symbol.value.symbol) {
            cdr(b) = value;
            return Error_ok;
        }
        bs = cdr(bs);
    }

    b = cons(symbol, value);
    cdr(env) = cons(b, cdr(env));

    return Error_ok;
}
