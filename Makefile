all:
	gcc basics.c builtins.c env.c eval.c -lreadline
