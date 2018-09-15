CC=gcc
CFLAGS=-I. -Wall -no-pie
DEPS = echobuiltin.h cdbuiltin.h pwdbuiltin.h pinfobuiltin.h remindme.h lsbuiltin.h clockbuiltin.h jobcontrol.h
OBJ = main.o echobuiltin.o cdbuiltin.o pwdbuiltin.o pinfobuiltin.o remindme.o lsbuiltin.o clockbuiltin.o jobcontrol.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

shell: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)