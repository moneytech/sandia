all:
	gcc basics.c file.c builtins.c env.c eval.c -lreadline

install:
	cp a.out /usr/local/bin/sandia
