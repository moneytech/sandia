#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <readline/readline.h>
#include <readline/history.h>

#include "basics.h"
#include "builtins.h"
#include "env.h"

int eval_expr (Atom expr, Atom env, Atom *result);
int lex (const char *str, const char **start, const char **end);
int parse_simple (const char *start, const char *end, Atom *result);
int read_expr (const char *input, const char **end, Atom *result);
int read_list (const char *start, const char **end, Atom *result);
void print_expr (Atom atom);

Atom copy_list (Atom list) {
    Atom a, p;

    if (nilp(list)) {
        return nil;
    }

    a = cons(car(list), nil);
    p = a;
    list = cdr(list);

    while (!nilp(list)) {
        cdr(p) = cons(car(list), nil);
        p = cdr(p);
        list = cdr(list);
    }

    return a;
}

int apply (Atom fn, Atom args, Atom *result) {
    Atom env, arg_names, body;

    if (fn.type == Atype_builtin) {
        return fn.value.builtin(args, result);
    }
    else if (fn.type != Atype_closure) {
        return Error_type;
    }

    env = env_create(car(fn));
    arg_names = car(cdr(fn));
    body = cdr(cdr(fn));

    /* bind the arguments */
    while (!nilp(arg_names)) {
        if (nilp(args)) {
            return Error_args;
        }

        env_set(env, car(arg_names), car(args));
        arg_names = cdr(arg_names);
        args = cdr(args);
    }

    if (!nilp(args)) {
        return Error_args;
    }

    /* evaluate the body */
    while (!nilp(body)) {
        Error err = eval_expr(car(body), env, result);
        if (err) {
            return err;
        }

        body = cdr(body);
    }

    return Error_ok;
}

#define symis(query) (strcmp(op.value.symbol, (query)) == 0)

int eval_expr (Atom expr, Atom env, Atom *result) {
    Atom op, args, p;
    Error err;

    if (expr.type == Atype_symbol) {
        return env_get(env, expr, result);
    } else if (expr.type != Atype_pair) {
        *result = expr;
        return Error_ok;
    }

    if (!listp(expr)) {
        return Error_syntax;
    }

    op = car(expr);
    args = cdr(expr);

    if (op.type == Atype_symbol) {
        if (symis("QUOTE")) {
            if (nilp(args) || !nilp(cdr(args))) {
                return Error_args;
            }

            *result = car(args);
            return Error_ok;
        }
        else if (symis("LAMBDA")) {
            if (nilp(args) || nilp(cdr(args))) {
                return Error_args;
            }

            return make_closure(env, car(args), cdr(args), result);
        }
        else if (symis("ENV")) {
          *result = env;
          return Error_ok;
        }
        else if (symis("DEFUN")) {
            Atom fun_name = car(car(args));
            Atom fun_args = cdr(car(args));
            Atom body = cdr(args);
            Atom fun_body = cons(make_sym("LAMBDA"), cons(fun_args, body));
            //Atom define_expr = cons(make_sym("DEFINE"), cons(fun_name, fun_body));
            Atom define_expr;
            Error err = make_closure(env, fun_args, body, &define_expr);
            if (err) {
                return err;
            }

            Atom sugar_res;
            err = eval_expr(define_expr, env, &sugar_res);
            if (err) {
                return err;
            }

            env_set(env, fun_name, sugar_res);
            *result = fun_name;
            return Error_ok;
        }
        else if (symis("DEFINE")) {
            Atom sym, val;

            if (nilp(args) || nilp(cdr(args)) || !nilp(cdr(cdr(args)))) {
                return Error_args;
            }

            sym = car(args);
            if (sym.type != Atype_symbol) {
                return Error_type;
            }

            err = eval_expr(car(cdr(args)), env, &val);
            if (err) {
                return err;
            }

            *result = sym;
            return env_set(env, sym, val);
        }
    }

    err = eval_expr(op, env, &op);
    if (err) {
        return err;
    }

    /* Evaluate arguments. */
    args = copy_list(args);
    p = args;
    while (!nilp(p)) {
        err = eval_expr(car(p), env, &car(p));
        if (err) {
            return err;
        }

        p = cdr(p);
    }


    return apply(op, args, result);
}

Atom make_builtin (Builtin fn) {
    Atom a = {
        .type = Atype_builtin,
        .value.builtin = fn
    };

    return a;
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
        default:
            printf("UNKNOWN");
            break;
    }
}

int lex (const char *str, const char **start, const char **end) {
    const char *ws = " \t\n";
    const char *delim = "() \t\n";
    const char *prefix = "()\'";

    str += strspn(str, ws);

    if (str[0] == '\0') {
        *start = *end = NULL;
        return Error_syntax;
    }

    *start = str;

    if (strchr(prefix, str[0]) != NULL) {
        *end = str + 1;
    }
    else {
        *end = str + strcspn(str, delim);
    }

    return Error_ok;
}

int parse_simple (const char *start, const char *end, Atom *result) {
    char *buf, *p;

    long val = strtol(start, &p, 10);

    /* is it an integer? */
    if (p == end) {
        result->type = Atype_int;
        result->value.integer = val;
        return Error_ok;
    }

    /* NIL or symbol */
    buf = calloc(1, end - start + 1);
    p = buf;

    while (start != end) {
        *p++ = toupper(*start);
        ++start;
    }

    *p = 0;

    if (strcmp(buf, "NIL") == 0) {
        *result = nil;
    }
    else {
        *result = make_sym(buf);
    }

    free(buf);

    return Error_ok;
}

int read_expr (const char *input, const char **end, Atom *result) {
    const char *token;
    Error err;

    err = lex(input, &token, end);
    if (err) {
        return err;
    }

    if (token[0] == '(') {
        return read_list(*end, end, result);
    }
    else if (token[0] == ')') {
        return Error_syntax;
    }
    else if (token[0] == '\'') {
        *result = cons(make_sym("QUOTE"), cons(nil, nil));
        return read_expr(*end, end, &car(cdr(*result)));
    }
    else {
        return parse_simple(token, *end, result);
    }
}

int read_list (const char *start, const char **end, Atom *result) {
    Atom p;

    *end = start;
    p = *result = nil;

    for (;;) {
        const char *token;
        Atom item;
        Error err;

        err = lex(*end, &token, end);
        if (err) {
            return err;
        }

        if (token[0] == ')') {
            return Error_ok;
        }

        if (token[0] == '.' && *end - token == 1) {
            /* improper list */
            if (nilp(p)) {
                return Error_syntax;
            }

            err = read_expr(*end, end, &item);
            if (err) {
                return err;
            }

            cdr(p) = item;

            /* read closing ')' */
            err = lex(*end, &token, end);
            if (!err && token[0] != ')') {
                err = Error_syntax;
            }

            return err;
        }

        err = read_expr(token, end, &item);
        if (err) {
            return err;
        }

        if (nilp(p)) {
            /* first item */
            *result = cons(item, nil);
            p = *result;
        }
        else {
            cdr(p) = cons(item, nil);
            p = cdr(p);
        }
    }
}

int main () {
    Atom env = env_create(nil);

#define X(name, op) env_set(env, make_sym(#op), make_builtin(builtin_##name));
    builtin_list
#undef X

        char *input;
    while ((input = readline("> ")) != NULL) {
        const char *p = input;
        Error err;
        Atom expr, result;

        err = read_expr(p, &p, &expr);
        if (!err)
            err = eval_expr(expr, env, &result);

        switch (err) {
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

        free(input);
    }

    return 0;
}
