#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "basics.h"

static Atom sym_table = { Atype_nil };

void error (const char *msg) {
    fprintf(stderr, "Error: %s\n", msg);
    exit(EXIT_FAILURE);
}

int listp (Atom expr) {
    while (!nilp(expr)) {
        if (expr.type != Atype_pair) {
            return 0;
        }
        expr = cdr(expr);
    }

    return 1;
}

Atom cons (Atom car_val, Atom cdr_val) {
    Atom p = {
        .type = Atype_pair,
        .value.pair = calloc(1, sizeof(struct Pair)),
    };

    if (!p.value.pair) {
        error("Could not allocate memory.");
    }

    car(p) = car_val;
    cdr(p) = cdr_val;

    return p;
}

Atom make_int (long x) {
    Atom a = {
        .type = Atype_int,
        .value.integer = x
    };

    return a;
}

Atom make_sym (const char *s) {
    Atom a, p;

    p = sym_table;
    while (!nilp(p)) {
        a = car(p);
        if (strcmp(a.value.symbol, s) == 0)
            return a;
        p = cdr(p);
    }

    a.type = Atype_symbol;
    a.value.symbol = strdup(s);
    sym_table = cons(a, sym_table);

    return a;
}

int make_closure (Atom env, Atom args, Atom body, Atom *result) {
    Atom p;

    if (!listp(args) || !listp(body)) {
        return Error_syntax;
    }

    /* check all arg names are symbols */
    p = args;
    while (!nilp(p)) {
        if (car(p).type != Atype_symbol) {
            return Error_type;
        }
        p = cdr(p);
    }

    *result = cons(env, cons(args, body));
    result->type = Atype_closure;

    return Error_ok;
}
