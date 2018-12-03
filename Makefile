source = payload.c socket.c acker.c saturateservo.c saturatr.c
objects = payload.o socket.o acker.o saturateservo.o
executables = saturatr

CC = gcc
CFLAGES = -g -Wall
LIBS = -lm

all: $(executables)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

saturatr: saturatr.o $(objects)
	$(CC) $(CFLAGS) -o $@ $+ $(LIBS)



-include depend

depend: $(source)
	$(CC) $(INCLUDES) -MM $(source) > depend

.PHONY: clean
clean:
	rm -f $(executables) depend *.o *.rpo
