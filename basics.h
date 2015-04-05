#ifndef BASICS_H
#define BASICS_H

struct Atom;

typedef int (*Builtin)(struct Atom args, struct Atom *result);

struct Atom {
    enum {
        Atype_nil,
        Atype_pair,
        Atype_symbol,
        Atype_int,
        Atype_builtin,
        Atype_closure,
    } type;

    union {
        struct Pair *pair;
        const char *symbol;
        long integer;
        Builtin builtin;
    } value;
};

struct Pair {
    struct Atom atom[2];
};

typedef struct Atom Atom;

#define car(p) ((p).value.pair->atom[0])
#define cdr(p) ((p).value.pair->atom[1])
#define nilp(atom) ((atom).type == Atype_nil)

static const Atom nil = { Atype_nil };

typedef enum {
    Error_ok = 0,
    Error_syntax,
    Error_unbound,
    Error_args,
    Error_type,
} Error;

void error (const char *msg);
int listp (Atom expr);
Atom cons (Atom car_val, Atom cdr_val);
Atom make_int (long x);
Atom make_sym (const char *s);
int make_closure (Atom env, Atom args, Atom body, Atom *result);

#endif
