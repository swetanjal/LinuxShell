CC=gcc
CFLAGS=-I. -Wall
DEPS = echobuiltin.h cdbuiltin.h pwdbuiltin.h execute.h pinfobuiltin.h remindme.h lsbuiltin.h clockbuiltin.h
OBJ = main.o echobuiltin.o cdbuiltin.o pwdbuiltin.o execute.o pinfobuiltin.o remindme.c lsbuiltin.c clockbuiltin.c

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

shell: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)