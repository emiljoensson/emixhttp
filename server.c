#include <stdio.h> // Declarations used in most input/output
#include <stdlib.h> // Defines several general purpose function
#include <string.h> // Defines variable type and various functions for manipulating arrays of characters
#include <sys/types.h> // Definitions of a number of data types used in system calls
#include <sys/socket.h> // Includes a number of definitions of structures needed for sockets
#include <netinet/in.h> // Constatns and structures needed for internet domain addresses

void error(char *msg);
void dostuff(int);

int main (int argc, char *argv[]) {

	/* Check for argument count */
	if (argc < 2)
		error("ERROR: no port specified");

	int sockfd, newsockfd;	// File descriptors, store the values returned by the socket system call and the accept system call
	int portno; 			// Port number for the server
	int clilen; 			// Size of the address of the client, needed for the accept system call
	int pid;				// Used for forking

	struct sockaddr_in serv_addr, cli_addr; // The structures containing an internet address

	/* Creates new socket, AF_INET for TCP/IPv4 and SOCK_STREAM for TCP. Error if faill */
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0)
		error("ERROR: Failed to open socket");

	bzero((char *) &serv_addr, sizeof(serv_addr));	// Sets all values in the buffer to zero
	portno = atoi(argv[1]); 						// Read port argument and put into portno

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
			dostuff(newsockfd);
			exit(0);
		}
		else
			close(newsockfd);
	}

	/* We'll never get past this point due to the infinite loop above */

	close(sockfd);
	return 0;
}

/* This function is called when a system call fails */
void error (char *msg) {
	perror(msg);
	exit(1);
}

void dostuff(int sock) {
	int valid = 1;		// Int to hold if request is valid or not (0/1)
	int n; 				// The return value for the read() and write() calls, contains the number of characters read or written
	char message[1024];	// The server reads characters from the socket connection in to this char array
	char response[1024];
	char *Resp_Header;
	char *Resp_ContentLength;
	char *Resp_ContentType;
	char *Resp_LastModified;
	char *Resp_Server = "\nServer: emix-http/0.1";
	char *Body ="\n\nHello World!";

	/* Reads from socket, read() will hold until theres something for it to read i the socket */
	bzero(message, sizeof(message));
	n = read(sock,message,sizeof(message));
	if (n < 0)
		error("ERROR: failed to read from socket");

	printf("Client Request: \n%s\n", message);

    /* Parse the client request */
    char* RequestLine = strtok(message, "\n");
    char* Req_Method = strtok(RequestLine, " ");
	char* Req_Path = strtok(NULL, " ");
	char* Req_Version = strtok(NULL, " ");
	if (Req_Path[0] =='/')			// Sets path to /index.html if / is requested
		Req_Path = "/index.html";

	/* Check if it's a valid HTTP 1.0 method, if not set valid to 0 else continues */
	if (strncmp(Req_Method,"GET",3) != 0 && strncmp(Req_Method,"HEAD",4) != 0 && strncmp(Req_Method,"POST",4) != 0) {
		valid = 0;
	}

	/* Check if it's not HTTP 1.0 or 1.1, if so set valid to 0, else continues */
	if (strncmp(Req_Version,"HTTP/1.0",8) != 0 && strncmp(Req_Version,"HTTP/1.1",8) != 0) {
		valid = 0;
	}

	if (valid == 1) {
		/* Check request method and compose response accordingly */
		if (strncmp(Req_Method,"GET",3) == 0 || strncmp(Req_Method,"HEAD",4) == 0) {
			Resp_Header="HTTP/1.0 200 OK";
			Resp_ContentLength = "\nContent-Length: 5";
			Resp_ContentType = "\nContent-Type: text/html";
			Resp_LastModified = "\nLast-Modified: Wed, 3 Feb 1992 16:00:00 GMT"; // TODO: get the actual date
			if (strncmp(Req_Method,"GET",3) == 0) {
				snprintf(response,sizeof(response), "%s%s%s%s%s%s",Resp_Header,Resp_ContentLength,Resp_ContentType,Resp_LastModified,Resp_Server,Body);
			} else {
				snprintf(response,sizeof(response), "%s%s%s%s%s",Resp_Header,Resp_ContentLength,Resp_ContentType,Resp_LastModified,Resp_Server);
			}
		} else if (strncmp(Req_Method,"POST",4) == 0 && valid == 1){
			Resp_Header="HTTP/1.0 501 Not Implemented";
			snprintf(response,sizeof(response), "%s%s",Resp_Header,Resp_Server);
		} else {
			Resp_Header="HTTP/1.0 400 Bad Request";
			snprintf(response,sizeof(response), "%s%s",Resp_Header,Resp_Server);
		}
	} else {
		Resp_Header="HTTP/1.0 400 Bad Request";
		snprintf(response,sizeof(response), "%s%s",Resp_Header,Resp_Server);
	}

	n = write(sock,response, strlen(response)); if (n < 0) error("ERROR: failed to write to socket");
}