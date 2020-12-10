CC = gcc
CFLAGS = -Wall

all: detector

detector: Asst3.c
	$(CC) $(CFLAGS) Asst3.c -o KKJserver -pthread -lm

# For debugging
debug: CFLAGS = -Wall -g -fsanitize=address,undefined -D DEBUG=1
debug: all

 
testing: CFLAGS = -D DEBUG=1
testing: all

clean:
	rm -f a.out detector Asst3 KKJserver *.o


