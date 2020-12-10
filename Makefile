CC = gcc
CFLAGS = -Wall

all: detector

detector: server.c
	$(CC) $(CFLAGS) server.c -o KKJserver -pthread -lm

# For debugging
debug: CFLAGS = -Wall -g -fsanitize=address,undefined -D DEBUG=1
debug: all

 
testing: CFLAGS = -D DEBUG=1
testing: all

clean:
	rm -f a.out detector Asst2 *.o


