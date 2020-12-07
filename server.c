#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <pthread.h>

#define BACKLOG 5

//server side

//struct made in example in class, might be useful
struct connection {
    struct sockaddr_storage addr;
    socklen_t addr_len;
    int fd;
};

int main(int argc, char** argv){

	if(argc != 3) {
		printf("error\n");
		return 0;
	}
	
	char* port = argv[1];

	// char from server
	char* start = "REG|13|Knock, knock.|";
	char* setup = "REG|4|Who.|";
	char* punch = "REG|30|I didn't know you were an owl";

	//socket creation
	int sockfd;
	if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
		printf("error, socket could not be created\n");
		return 0;
	}
	
	
	// this is flimsy, i'm not even sure if we need this
	struct addrinfo hint, *address_list;
	memset(&hint, 0, sizeof(struct addrinfo));
    	hint.ai_family = AF_UNSPEC;
    	hint.ai_socktype = SOCK_STREAM;
    	hint.ai_flags = AI_PASSIVE;
	if(getaddrinfo(NULL, port, &hint, &address_list) != 0){
		printf("error\n");
		return 0;
	}
	
	// i have no idea how to do bind
	/*if(bind(sockfd, addr->ai_addr, addr->ai_addrlen) < 0) {
		printf("error, socket bind failed\n");
		return 0;
	}*/

	// this is for listen
	if(listen(sockfd, BACKLOG) < 0) {
		printf("error, listen failed\n");
		return 0;
	}
	
	// this is for accept
	struct connection *con;
	con = malloc(sizeof(struct connection));
	con->addr_len = sizeof(struct sockaddr_storage);
	con->fd = accept(sockfd, (struct sockaddr *) &con->addr, &con->addr_len);
	if(con->fd == -1) {
		printf("error, accept failed\n");
		return 0;
	}
	
	// this part is for the actual read in
	int valread;
	char buffer[1024] = {0};
	
	//read in first 4 characters which should be REG| or ERR|
	valread = read(con->fd, buffer, 4);
	char* temp = malloc(sizeof(char) * 5);
	memcpy(temp, buffer, valread);
	temp[valread] = '\0'
	//read rest
	if(strcmp(temp, "REG|") == 0) {
		
	} else if (strcmp(temp, "ERR|") == 0) {
	
	}


	printf("aids\n");



	return 0;


}
