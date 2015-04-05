#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "basics.h"

static Atom sym_table = { Atype_nil };

void error (const char *msg) {
    fprintf(stderr, "Error: %s\n", msg);
    exit(EXIT_FAILURE);
}

void print_err (Error e, Atom result) {
  switch (e) {
  case Error_ok:
    print_expr(result);
    putchar('\n');
    break;
  case Error_syntax:
    puts("Syntax error");
    break;
  case Error_unbound:
    puts("Symbol not bound");
    break;
  case Error_args:
    puts("Wrong number of arguments");
    break;
  case Error_type:
    puts("Wrong type");
    break;
  }
}

void print_expr (Atom atom) {
    switch (atom.type) {
        case Atype_nil:
            printf("NIL");
            break;
        case Atype_pair:
            putchar('(');
            print_expr(car(atom));
            atom = cdr(atom);
            while (!nilp(atom)) {
                if (atom.type == Atype_pair) {
                    putchar(' ');
                    print_expr(car(atom));
                    atom = cdr(atom);
                } else {
                    printf(" . ");
                    print_expr(atom);
                    break;
                }
            }
            putchar(')');
            break;
        case Atype_symbol:
            printf("%s", atom.value.symbol);
            break;
        case Atype_int:
            printf("%ld", atom.value.integer);
            break;
        case Atype_builtin:
            printf("#<BUILTIN:%p>", atom.value.builtin);
            break;
        case Atype_closure:
            print_expr(cons(make_sym("LAMBDA"), cdr(atom)));
            break;
        default:
            printf("UNKNOWN");
            break;
    }
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
