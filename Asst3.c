#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <pthread.h>
#include <ctype.h>

#define BACKLOG 10
#define BUFSIZE 16

int server(char *port);
void *knockKnock(void *arg);
int readRemote(int fd, char** str_ptr);

int main(int argc, char **argv)
{
	// argv[1] is the port number
	if (argc != 2) {
		printf("Usage: %s {port}\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	// pass port number into function
	server(argv[1]);
	return EXIT_SUCCESS;
}


/* int server(char* port)
 * Inputs: port (name of port entered on command line)
 * Outputs: int (never gets to return if successful
 * 		returns EXIT_FAILURE if an error occurs)
 * Functions: getaddrinfo, socket, bind, freeadrinfo, accept
 * 		pthread_create, pthread_detach
 */
int server(char *port){
	
	// Set addressing info
	struct addrinfo hint, *address_list, *addr;
	memset(&hint, 0, sizeof(struct addrinfo));
	hint.ai_family = AF_UNSPEC;
	hint.ai_socktype = SOCK_STREAM;
	hint.ai_flags = AI_PASSIVE;

	// Create and format addressing info using given port on localhost
	int err = getaddrinfo(NULL, port, &hint, &address_list);
	if (err != 0){
		printf("getaddrinfo ERROR: %s\n", gai_strerror(err));
		return EXIT_FAILURE;
	}

	// Create a socket using address info
	int sockfd;
	for (addr = address_list; addr != NULL; addr = addr->ai_next) {
		// Attempt creation of socket
		if((sockfd = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol)) == -1) {
			// Try the next address if we can't create the socket
			continue;
		}

		// Try to bind and listen
		if((bind(sockfd, addr->ai_addr, addr->ai_addrlen) == 0) && listen(sockfd, BACKLOG) == 0) {
			// On success, break out of the loop
			break;
		}
		
		// Socket didn't work, try next
		close(sockfd);

	}

	// Ended loop without succeeding
	if(addr == NULL) {
		printf("ERROR, can't bind any socket\n");
		freeaddrinfo(address_list);
		return EXIT_FAILURE;
	}

	// No longer need list of addresses to connect to
	freeaddrinfo(address_list);
	
	printf("Waiting...\n");


	// We have a socket bound and listening
	// Ready to call accept
	pthread_t tid;
	while (1) {
		// Wait an incoming connection with accept
		// Don't care who we're talking to:
		// Pass connection as NULL
		int confd = accept(sockfd, NULL, NULL);
		if(confd == -1) {
			printf("ERROR, accept failed\n");
			continue;
		}
		
		// Successfully connected, create thread to joke with
		err = pthread_create(&tid, NULL, knockKnock, &confd);
		if (err != 0) {
			perror("Could not create thread:");
			close(confd);
			continue;
		}
		
		// Detach thread and wait for next connection
		pthread_detach(tid);
	}

}


/* void errHandler(int fd, char *msg, int retVal, int lineNum)
 * Inputs: fd (file descriptor to read from),
 *         *msg (message to be read),
 *         retVal (determines indentifier in function),
 *         lineNum (line number function is working with)
 * Output: int (-1 if error, 1 otherwise)
 */
int errHandler(int fd, char *msg, int retVal, int lineNum) {
	int originalLine = lineNum;
	char *identifier;
	
	// Format error
	if (retVal == -1) identifier = "FT";
	else if (retVal == -2) {
		char cmpStr[5] = "MXCT";
		int validErr = 0;
		
		// Changing X for line number
		cmpStr[1] = '0' - 1 + lineNum;
		lineNum--;
		
		// Checks for content error
		if (strcmp(msg, cmpStr) == 0) {
			identifier = "CT";
			validErr = 1;
		} 

		cmpStr[2] = 'L';
		cmpStr[3] = 'N';

		// Checks for length error
		if (strcmp(msg, cmpStr) == 0) {
			identifier = "LN";
			validErr = 1;
		}

		cmpStr[2] = 'F';
		cmpStr[3] = 'T';

		// Checks for format error
		if (strcmp(msg, cmpStr) == 0) {
			identifier = "FT";
			validErr = 1;
		}
		
		free(msg);

		// If none of the above, it's a format error
		if (!validErr) {
			lineNum++;
			identifier = "FT";
		}
	
	} else if (retVal == -3) {
		identifier = "CT";
	} else if (strlen(msg) != retVal) {
		identifier = "LN";
		free(msg);
	} else return 0;

	if (originalLine == lineNum) {
		// Writing correct error message
		char errToSend[10] = "ERR|MXYY|\0";
		errToSend[5] = '0' + lineNum;
		errToSend[6] = identifier[0];
		errToSend[7] = identifier[1];
		write(fd, errToSend, strlen(errToSend));
		printf("Sent Error Message: %s\n", errToSend);
	} else {
		printf("Received Error: M%d%s\n", lineNum, identifier);	
	}
	return -1;
 
}


/* int isPunct(char c)
 * Input: c (character in question) 
 * Output: int (1 if character is punctuation
 * 		0 if character is not)
 */
int isPunct(char c) {
	return c == '!' || c == '?' || c == '.';
}


/* void *knockKnock(void *arg)
 * Input: *arg (argument passed in by pthread_create) 
 * Output: NULL
 * Function: Write, readRemote, errHandler
 */
void *knockKnock(void *arg) {
	int fd = *((int *)arg);
	char *msg;
	int err;

	// Line 1
	char *line0 = "REG|13|Knock, knock.|";
	write(fd, line0, strlen(line0));
	printf("Sent Message: %s\n", line0);
	
	// Receive message from remote
	// Should be Who's There?
	err = readRemote(fd, &msg);
	// Check for format/length errors
	err = errHandler(fd, msg, err, 1);
	// Check for content errors	
	if (err >= 0 && strcmp(msg, "Who's there?") != 0) {
		err = errHandler(fd, NULL, -3, 1);
	}
	if (err < 0) {
		// Error, exit
		close(fd);
		return NULL;
	}
	printf("Received message: %s\n", msg);
	free(msg);


	// Send line 2
	char *line2 = "REG|4|Who.|";
	write(fd, line2, strlen(line2));
	printf("Sent Message: %s\n", line2);
	
	// Receive message from remote
        // Should be Who, who?
	err = readRemote(fd, &msg);
	// Check for format/length errors
	err = errHandler(fd, msg, err, 3);
	// Check for content errors   
	if (err >= 0 && strcmp(msg, "Who, who?") != 0) {
		err = errHandler(fd, NULL, -3, 3);
	}
	if (err < 0) {
		// Error, exit
		close(fd);
		return NULL;
	}
	printf("Received message: %s\n", msg);
	free(msg);

	// Send line 4
	char *line4 = "REG|30|I didn't know you were an owl!|";
	write(fd, line4, strlen(line4));
	printf("Sent Message: %s\n", line4);
	// Receive message from remote
	err = readRemote(fd, &msg);
	// Check for format/length errors
	err = errHandler(fd, msg, err, 5);
	// Check for content errors   
	if (err >= 0) {
		int i;
		for (i = 0; i < strlen(msg) - 1; i++) {
			if (!isalpha(msg[i]) && !isspace(msg[i])
			    && !isPunct(msg[i]) && msg[i] != ','
			    && msg[i] != '-' && msg[i] != '\''
			    && msg[i] != '(' && msg[i] != ')') {
				err = -1;
				break;
			}
		}
		if (!isPunct(msg[i])) err = -1;
		if (err < 0) errHandler(fd, NULL, -3, 5);
	}
	if (err < 0) {
		// Error, exit
		close(fd);
		return NULL;
	}

	printf("Received message: %s\n", msg);
	free(msg);
	close(fd);

	return NULL;
}

/* int readRemote(int fd, char** str_ptr)
 * Inputs: fd (file descriptor to read from), 
 *         str_ptr (pointer to a str to write to)
 * Outputs: int (Length if correct format, -1 if format error
 *          -2 if error msg but formatted correctly
 * Function: Read
 */
int readRemote(int fd, char** str_ptr) {
	
	int bytes, type = 0, errVal = 0;
	
	// Create buffer
	char buf[1];
	int size = BUFSIZE;
	char *currStr = malloc(size);
	int used = 0;
	
	int msgLen = -1;

	while ((bytes = read(fd, buf, 1)) > 0) {
		// Note: This inner loop only has one iteration
		// It is only here in case we change the implementatin
		for (int i = 0; i < bytes; i++) {
			// Insert current char into currStr buffer
			if (used == size) {
				size *= 2;
				currStr = realloc(currStr, size);
			}
			currStr[used++] = buf[i];
			
			/* type corresponds to the part of the string
			 * we are reading in:
			 * 0 - Undecided (1st part, ERR or REG)
			 * 1 - REG string, reading length
			 * 2 - REG/ERR string, reading content
			 * 3 - Finished string - removed
			 * -1 - ERR string
			*/ 
			if (type == -1) {
				errVal = -2;
				type = 2;
			}
			if (type == 0) { // Type undecided (yet)
				if (used == 4) {
					// 4 chars in buffer
					// Check the type we should set or return error
					if (memcmp(currStr, "REG|", 4) == 0) type = 1;
					else if (memcmp(currStr, "ERR|", 4) == 0) type = -1;
					else return -1;
					used = 0;
				} else if (buf[i] == '|') return -1;
			} else if (type == 1) { 
				// Reading in msg len
				// Read in numbers until we hit '|'
				if (buf[i] == '|') {
					if (used == 1) return -1;
					// Set length
					type = 2;
					currStr[used - 1] = '\0';
					msgLen = atoi(currStr);
					used = 0;
				} else if (!isdigit(buf[i])) return -1;
			} else if (type == 2) { // Read in actual msg
				if (buf[i] == '|') {
					// Make string shortest possible length and set end to '\0'
					currStr = realloc(currStr, used);
					currStr[used - 1] = '\0';
					*str_ptr = currStr;
					return (errVal == 0) ? msgLen : errVal;
				} else if (used == msgLen + 1) return -1;
			}
		}
	}
	
	// Did not get a fully formatted message
	return -1;
}

