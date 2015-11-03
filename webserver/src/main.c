#include <stdio.h>			// Declarations used in most input/output
#include <stdlib.h>			// Defines several general purpose function
#include <string.h>			// Defines variable type and various functions for manipulating arrays of characters
#include <unistd.h>			// Needed to use getopt() etc to handle command line arguments better
#include <time.h>			// Needed when checking Last-Modified (ctime())
#include <sys/types.h>		// Definitions of a number of data types used in system calls
#include <sys/socket.h>		// Includes a number of definitions of structures needed for sockets
#include <sys/stat.h>		// stat() to obtain information about file
#include <netinet/in.h>		// Constants and structures needed for internet domain addresses

#include "error.h"
#include "new_connection.h"

int main (int argc, char *argv[]) {

	int sockfd, newsockfd;	// File descriptors, store the values returned by the socket system call and the accept system call
	int portno; 			// Port number for the server
	int clilen; 			// Size of the address of the client, needed for the accept system call
	pid_t pid;				// Used for forking
	int status;				// Used for forking

	/* Check for arguments */
	if (argc < 2)
		error("Invalid arguments. Please specify port with [-p N] (-h for help)");

	if(strncmp(argv[1], "-h", 2) == 0) {
		printf("Please specify port with [-p N]\n");
		return 0; 
	} else if(strncmp(argv[1], "-p", 2) == 0) {
		if(argv[2] != NULL) {
			if(atoi(argv[2]) > 1 && atoi(argv[2]) < 65535) {
				portno = atoi(argv[2]);
			} else {
				printf("Invalid port number, should be between 1 and 65535.\n");
				return 0;
			}
		} else {
			printf("No port number specified.\n");
			return 0;
		}
	} else {
		printf("Invalid arguments. Please specify port with [-p N] (-h for help)\n");
		return 0;
	}

	struct sockaddr_in serv_addr, cli_addr; // The structures containing an internet address

	/* Creates new socket, AF_INET for TCP/IPv4 and SOCK_STREAM for TCP. Error if faill */
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0)
		error("ERROR: Failed to open socket");

	bzero((char *) &serv_addr, sizeof(serv_addr));	// Sets all values in the buffer to zero

	serv_addr.sin_family = AF_INET; 		// Code for address family 
	serv_addr.sin_port = htons(portno); 	// htons convert port number in host byte order to a port number in network byte order
	serv_addr.sin_addr.s_addr = INADDR_ANY; // IP address of the host, we take the IP of the machine that runs the server

	/* Binds a socket to an address (ip and port), print error if fail (e.g port already in use) */
	if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
			 error("ERROR: failed to bind the socket");

	listen(sockfd, 5); // Listen on the socket for connections. 5 backlog queue limit

	/* Infinite loop that waits for a client to connect with accept(), then forks */
	clilen = sizeof(cli_addr);
	while (1) {
		newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
		if (newsockfd < 0)
			error("ERROR: failed to accept connection");

		pid = fork();
		if (pid < 0)
			error("ERROR: failed to fork");
		if (pid == 0) {
			close(sockfd);
			new_connection(newsockfd);
			exit(0);
		}
		else
			close(newsockfd);

		pid = wait(&status); // Waiting for child process to die, plz no zombie apocalypse 
	}

	/* We'll never get past this point due to the infinite loop above */

	close(sockfd);
	return 0;
}
