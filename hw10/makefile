SHELL = /bin/bash
CC = gcc
CFLAGS = -g
DBG_CFLAGS = -g --static
SRC = $(wildcard *.c)
EXE = $(patsubst %.c, %, $(SRC))

all: ${EXE}
%:	%.c
	${CC} ${DBG_CFLAGS} $@.c -o $@
clean:
	rm -f ${EXE}
	