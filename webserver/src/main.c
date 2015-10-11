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
	int pid;				// Used for forking

	/* TODO #1 - Read command line arguments:
		-h (prints help text)
		-p 80 (set portno to 80)
		See lab specification section 2.5
	*/

	/* Check for argument count (temporary fix, remove when fixing TODO #1) */
	if (argc < 2)
		error("ERROR: no port specified");
	portno = atoi(argv[1]); // Read port argument and put into portno

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

		waitpid(-1, NULL, WNOHANG); // Let child process die (we don't want the zombie problem)
	}

	/* We'll never get past this point due to the infinite loop above */

	close(sockfd);
	return 0;
}