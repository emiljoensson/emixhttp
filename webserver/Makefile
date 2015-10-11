CC = gcc
LD = gcc
INC_DIR = include
SRC_DIR = src
OBJ_DIR = objects
CFLAGS = -g -Wall -I$(INC_DIR)
LDFLAGS = 
SRCS = $(SRC_DIR)/main.c $(SRC_DIR)/error.c $(SRC_DIR)/new_connection.c
OBJS = $(OBJ_DIR)/main.o $(OBJ_DIR)/error.o $(OBJ_DIR)/new_connection.o
PROG = emixhttp
RM = /bin/rm

# all: emixhttp

all: $(PROG)

# emixhttp: main.o error.o new_connection.o
# 	gcc -o emixhttp main.o error.o new_connection.o

$(PROG): $(OBJS)
	$(LD) $(LDFLAGS) $(OBJS) -o $(PROG)

#main.o: main.c
#	gcc -g -c main.c

#error.o: error.c
#	gcc -g -c error.c

#new_connection.o: new_connection.c
#	gcc -g -c new_connection.c

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

#clean:
#	rm main.o error.o new_connection.o

clean:
	$(RM) $(PROG) $(OBJS)