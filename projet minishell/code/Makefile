CFLAGS=-Wall -g
LDFLAGS=
TESTDIR=tests

all: testlex minishell

analex.o: analex.c analex.h
	gcc ${CFLAGS} -c $<

minishell.o: minishell.c analex.h
	gcc ${CFLAGS} -c $<

testlex.o: testlex.c analex.h
	gcc ${CFLAGS} -c $<

testlex: testlex.o analex.o
	gcc ${LDFLAGS} -o $@ $^

minishell: minishell.o analex.o
	gcc ${LDFLAGS} -o $@ $^

test: minishell ${TESTDIR}/*.ms
	./runtest.sh ${TESTDIR}/*.ms

.PHONY clean:
	rm -rf *.o *~ .minishell.tmp testlex minishell
