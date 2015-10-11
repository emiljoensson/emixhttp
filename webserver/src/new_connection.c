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

void new_connection(int sock) {
	int valid = 1;		// Int to hold if request is valid or not (0/1)
	int n; 				// The return value for the read() and write() calls, contains the number of characters read or written
	char message[1024];	// The server reads characters from the socket connection in to this char array
	char response[1024];
	char *Resp_Header;
	int Resp_ContentLength;
	char *Resp_ContentType;
	char *Resp_LastModified;
	char *Resp_Server = "emix-http/0.1";

	/* Reads from socket, read() will hold until theres something for it to read i the socket */
	bzero(message, sizeof(message));
	n = read(sock,message,sizeof(message));
	if (n < 0)
		error("ERROR: failed to read from socket");

	/* DEBUGGING , CONSOLE OUTPUT */
	printf("Client Request: \n%s\n", message);

    /* Parse the client request */
	char* RequestLine = strtok(message, "\n");
	char* Req_Method = strtok(RequestLine, " ");
	char* Req_Path = strtok(NULL, " ");
	char* Req_Version = strtok(NULL, " ");
	if (Req_Path[0] =='/' && strlen(Req_Path) == 1) // Sets path to /index.html if / is requested
		Req_Path = "index.html";

	/* Maka a struct that stores the attributes of the file */
	struct stat attr;
	int FileLength;
	FILE *file;

	/* Check if it's a valid HTTP 1.0 method, if not set valid to 0 else continues */
	if (strncmp(Req_Method,"GET",3) != 0 && strncmp(Req_Method,"HEAD",4) != 0 && strncmp(Req_Method,"POST",4) != 0) {
		valid = 0;
	}

	/* Check if it's not HTTP 1.0 or 1.1, if so set valid to 0, else continues */
	if (strncmp(Req_Version,"HTTP/1.0",8) != 0 && strncmp(Req_Version,"HTTP/1.1",8) != 0) {
		valid = 0;
	}

	int sendBody = 1;

	if (valid == 0) { // If it's an invalid HTTP request
		Resp_Header="HTTP/1.0 400 Bad Request";
		snprintf(response,sizeof(response), "%s%s",Resp_Header,Resp_Server);
		Req_Path = "400.html";
	} else { // If it's a valid HTTP request

		/* Check request method and compose response accordingly */
		if (strncmp(Req_Method,"GET",3) == 0 || strncmp(Req_Method,"HEAD",4) == 0) { //If it's GET or HEAD

			/* Checking if we find the requested file */

			if (access(Req_Path, F_OK) == 0 && access(Req_Path, R_OK) == 0) { // Found the file and can read (200 OK)
				/* Figuring out Content-Length (check length of file) */
				stat(Req_Path, &attr);
				FileLength = (int)attr.st_size;
				/* Setting the headers */
				Resp_Header="HTTP/1.0 200 OK";
				Resp_ContentLength = FileLength;
				Resp_ContentType = "text/html";
				Resp_LastModified = ctime(&attr.st_mtime);
				snprintf(response,sizeof(response), "%s\nContent-Length: %d\nContent-Type: %s\nLast-Modified: %sServer: %s",Resp_Header,Resp_ContentLength,Resp_ContentType,Resp_LastModified,Resp_Server);
				if (strncmp(Req_Method,"HEAD",4) == 0) {
					sendBody = 0;
				}
			} else if (access(Req_Path, F_OK) == 0 && access(Req_Path, R_OK) == -1) { // Found the file but can't read (403)
				Resp_Header="HTTP/1.0 403 Forbidden";
				Req_Path = "403.html";
				snprintf(response,sizeof(response), "%s\nServer: %s",Resp_Header,Resp_Server);
			} else { // Did NOT find the file (404 Not Found)
				Resp_Header="HTTP/1.0 404 Not Found";
				Req_Path = "404.html";
				snprintf(response,sizeof(response), "%s\nServer: %s",Resp_Header,Resp_Server);
			}
		} 

		else if (strncmp(Req_Method,"POST",4) == 0 && valid == 1){ // If POST, return 501
			Resp_Header="HTTP/1.0 501 Not Implemented";
			Req_Path = "501.html";
			snprintf(response,sizeof(response), "%s\nServer: %s",Resp_Header,Resp_Server);
		}

		else { // If it's not GET, HEAD or POST, return 400
			Resp_Header="HTTP/1.0 400 Bad Request";
			Req_Path = "400.html";
			snprintf(response,sizeof(response), "%s\nServer: %s",Resp_Header,Resp_Server);
		}
	}

	/* DEBUGGING , CONSOLE OUTPUT */
	printf("\nOur response headers: %s\n", response);

	/* Writing to the socket (sending the response) NOTE: Only the headers so far */
	n = write(sock,response, strlen(response)); if (n < 0) error("ERROR: failed to write to socket");

	/* Sending the body (content of Req_Path) */
	if (sendBody == 1) {

		file = fopen(Req_Path, "r");
		stat(Req_Path, &attr);
		FileLength = (int)attr.st_size;

		char sdbuf[8196];
	    bzero(&sdbuf, sizeof(sdbuf));
	    int bytes = 0;
	    fseek(file, 0, SEEK_END);
	    rewind(file);
	    int line;
	    write(sock,"\n\n",4); // Making two line breaks between headers and body
	    while(bytes < FileLength){
	        line = fread(sdbuf, 1, sizeof(sdbuf), file);
	        bytes += line;
	        write(sock, sdbuf, line);
	        bzero(&sdbuf, sizeof(sdbuf));
	    }
		fclose(file);
	}
}