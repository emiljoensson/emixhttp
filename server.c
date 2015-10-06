#include <stdio.h> // declarations used in most input/output
#include <stdlib.h> // defines several general purpose function
#include <string.h> // defines variable type and various functions for manipulating arrays of characters
#include <sys/types.h> // definitions of a number of data types used in system calls
#include <sys/socket.h> // includes a number of definitions of structures needed for sockets
#include <netinet/in.h> // constatns and structures needed for internet domain addresses

void dostuff(int);
void error(char *msg);

int main (int argc, char *argv[]) {

	/* check for argument count */
	if (argc < 2)
		error("ERROR: no port specified");

	int sockfd, newsockfd; // file descriptors, store the values returned by the socket system call and the accept system call
	int portno; // port number for the server
	int clilen; // size of the address of the client, needed for the accept system call
	int pid;

	struct sockaddr_in serv_addr, cli_addr; // the structures containing an internet address

	/* creates new socket, AF_INET for TCP/IPv4 and SOCK_STREAM for TCP. Error if faill */
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0)
		error("ERROR: Failed to open socket");

	bzero((char *) &serv_addr, sizeof(serv_addr)); // sets all values in the buffer to zero
	portno = atoi(argv[1]); // read port argument and put into portno

	serv_addr.sin_family = AF_INET; // code for address family 
	serv_addr.sin_port = htons(portno); // htons convert port number in host byte order to a port number in network byte order
	serv_addr.sin_addr.s_addr = INADDR_ANY; // IP address of the host, we take the IP of the machine that runs the server

	/* binds a socket to an address (ip and port), print error if fail (i.e port already in use) */
	if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
			 error("ERROR: failed to bind the socket");

	listen(sockfd, 5); // listen on the socket for connections. 5 backlog queue limit

	/* will wait for a client to connect with accept() */
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
			dostuff(newsockfd);
			exit(0);
		}
		else
			close(newsockfd);
	}

	/* we'll never get past this point due to the infinite loop above */

	close(sockfd);
	return 0;
}

/* This function is called when a system call fails */
void error (char *msg) {
	perror(msg);
	exit(1);
}

void dostuff(int sock) {
	int n; // the return value for the read() and write() calls, contains the number of characters read or written
	char buffer[256]; // the server reads characters from the socket connection in to this buffer

	/* reads from socket, will block until theres something for it to read i the socket.
		will read the characters in the socket up to 255. error if fail*/
	bzero(buffer, 256);
	n = read(sock,buffer,255);
	if (n < 0)
		error("ERROR: failed to read from socket");
	printf("Here is the message: %s", buffer);

	/* write to socket so client can read, error if fail */
	n = write(sock,"I got your message", 18);
	if (n < 0)
		error("ERROR: failed to write to socket");
}