#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <pthread.h>
#include <ctype.h>

#define BACKLOG 5

//server side

//struct made in example in class, might be useful
struct connection {
    struct sockaddr_storage addr;
    socklen_t addr_len;
    int fd;
};

int server(char *port);
void *echo(void *arg);

int main(int argc, char **argv)
{
    (void) server(argv[1]);
    return EXIT_SUCCESS;
}


int server(char *port){
/*
	if(argc != 3) {
		printf("error\n");
		return 0;
	}
	*/
	//service given in command line
	//char* port = argv[1];

	// char from server
	/*
	char* start = "REG|13|Knock, knock.|";
	char* setup = "REG|4|Who.|";
	char* punch = "REG|30|I didn't know you were an owl";
*/
	char* who = "Who's there?";


	struct addrinfo hint, *address_list, *addr;
        memset(&hint, 0, sizeof(struct addrinfo));
        hint.ai_family = AF_UNSPEC;
        hint.ai_socktype = SOCK_STREAM;
        hint.ai_flags = AI_PASSIVE;
        if(getaddrinfo(NULL, port, &hint, &address_list) != 0){
                printf("error\n");
                return 0;
        }

	//socket creation
	int sockfd;
	for (addr = address_list; addr != NULL; addr = addr->ai_next) {
		if((sockfd = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol)) == -1) {
			printf("error, socket could not be created\n");
			return 0;
		}

		if((bind(sockfd, addr->ai_addr, addr->ai_addrlen) == 0) && listen(sockfd, BACKLOG) == 0) {
			break;
		}

		close(sockfd);

	}

	if(addr == NULL) {
		printf("error, can't bind\n");
		return 0;
	}

	freeaddrinfo(address_list);
	printf("waiting\n");	

	// this is for accept
	struct connection *con;
	pthread_t tid;
	for(;;){
		con = malloc(sizeof(struct connection));
		con->addr_len = sizeof(struct sockaddr_storage);
		con->fd = accept(sockfd, (struct sockaddr *) &con->addr, &con->addr_len);
		printf("%d\n", con->fd);
		if(con->fd == -1) {
			printf("error, accept failed\n");
			return 0;
		}

		pthread_create(&tid, NULL, echo, con);
		
	}


	// this part is for the read in
	int valread;
	char buffer[1024] = {0};
	
	//read in first 4 characters which should be REG| or ERR|
	valread = read(con->fd, buffer, 4);
	char* temp = malloc(sizeof(char) * 5);
	memcpy(temp, buffer, valread);
	temp[valread] = '\0';
	//read rest
	if(strcmp(temp, "REG|") == 0) {
		valread = read(con->fd, buffer, 1);
		temp[0] = buffer[0];
		valread = read(con->fd, buffer, 1);
                temp[1] = buffer[0];
		valread = read(con->fd, buffer, 1);
                if(buffer[0] != '|'){ //wrong character
			if(isdigit(buffer[0])) {
				//2 digits in a row, certain error
			} else {
				//some other character besides |
			}
		} else {
			temp[2] = '\0';
			int length = atoi(temp);
			if(length != 12) { //reference to line: Who's there?
				//wrong length
			} else {
				int i = 0;
				while (i < 12) {
					valread = read(con->fd, buffer, 1);
					if(buffer[0] != who[i]){
						//message content don't match
					}
					i++;	
				}
				valread = read(con->fd, buffer, 1);
				if(buffer[0] != '|') {
					//missing last |
				}
			}	
		}	
	} else if (strcmp(temp, "ERR|") == 0) {
	
	}

	return 1;
}


//this is from class example
void *echo(void *arg) {
   	char host[100], port[10], buf[101];
    	struct connection *c = (struct connection *) arg;
    	int error, nread;

    	error = getnameinfo((struct sockaddr *) &c->addr, c->addr_len, host, 100, port, 10, NI_NUMERICSERV);
    
    	if (error != 0) {
        	fprintf(stderr, "getnameinfo: %s", gai_strerror(error));
        	close(c->fd);
        	return NULL;
    	}

    	printf("[%s:%s] connection\n", host, port);

    	while ((nread = read(c->fd, buf, 100)) > 0) {
        	buf[nread] = '\0';
        	printf("[%s:%s] read %d bytes |%s|\n", host, port, nread, buf);
    	}

    	printf("[%s:%s] got EOF\n", host, port);

    	close(c->fd);
    	free(c);
    	return NULL;
}

