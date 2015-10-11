CC = gcc
LD = gcc
INC_DIR = include
SRC_DIR = src
OBJ_DIR = objects
CFLAGS = -g -Wall -I$(INC_DIR)
LDFLAGS = 
SRCS = $(SRC_DIR)/main.c $(SRC_DIR)/new_connection.c
OBJS = $(OBJ_DIR)/main.o $(OBJ_DIR)/new_connection.o
PROG = emixhttp
RM = /bin/rm

# all: emixhttp

all: $(PROG)

# greetings: main.o new_connection.o
# 	gcc -o greetings main.o new_connection.o

$(PROG): $(OBJS)
	$(LD) $(LDFLAGS) $(OBJS) -o $(PROG)

#main.o: main.c
#	gcc -g -c new_connection.c

#new_connection.o: new_connection.c
#	gcc -g -c new_connection.c

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

#clean:
#	rm main.o new_connection.o

clean:
	$(RM) $(PROG) $(OBJS)
