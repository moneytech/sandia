#ifndef ENV_H
#define ENV_H

#include "basics.h"

Atom env_create (Atom parent);
int env_get (Atom env, Atom symbol, Atom *result);
int env_set (Atom env, Atom symbol, Atom value);

#endif
