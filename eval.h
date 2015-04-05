#ifndef EVAL_H
#define EVAL_H

#include "basics.h"

int eval_expr (Atom expr, Atom env, Atom *result);
int lex (const char *str, const char **start, const char **end);
int parse_simple (const char *start, const char *end, Atom *result);
int read_expr (const char *input, const char **end, Atom *result);
int read_list (const char *start, const char **end, Atom *result);

#endif
