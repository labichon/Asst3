#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <pthread.h>


//this is the client side
//I'm fairly certain there's an error here

int main(int argc, char** argv) {
	
	/*
	char* start = "REG|12|Who's there?|";
	char* setup = "REG|9|Who, who?|";
	char* disgust = "REG|4|Ugh.|";
*/
	struct addrinfo hint, *address_list, *addr;
	memset(&hint, 0, sizeof(hint));
    	hint.ai_family = AF_UNSPEC;
   	hint.ai_socktype = SOCK_STREAM;
    	
	//argv[1] remote host
	//argv[2] service, same as port in server.c
	getaddrinfo(argv[1], argv[2], &hint, &address_list);

	int sockfd;
	for (addr = address_list; addr != NULL; addr = addr->ai_next) {
		sockfd = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);
		printf("%d\n", sockfd);
		if(sockfd < 0){
			continue;
		}
		if (connect(sockfd, addr->ai_addr, addr->ai_addrlen) == 0) {
			break;
		}
		close(sockfd);
	}

	if (addr == NULL) {
		printf("error, socket can't be created\n");
		return 0;
	}


	freeaddrinfo(address_list);

	close(sockfd);

	return EXIT_SUCCESS;

}
