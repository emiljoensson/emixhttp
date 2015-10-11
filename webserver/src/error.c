#include <stdio.h>
#include <stdlib.h>

#include <error.h>

/* This function is called when a system call fails */
void error (char *msg) {
	perror(msg);
	exit(1);
}