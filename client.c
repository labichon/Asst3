#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <pthread.h>


//this is the client side

int main(int argc, char** argv) {
	
	char* start = "REG|12|Who's there?|";
	char* setup = "REG|9|Who, who?|";
	char* disgust = "REG|4|Ugh.|";

	int sock;
	if((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0){
		printf("error, socket not created\n");
		return 0;
	}



}
