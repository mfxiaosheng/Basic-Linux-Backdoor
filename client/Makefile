CC = gcc
CFLAGS = -DDEBUG -g -Wall
SRC_DIR = ./src
INC_DIR = ./inc
OBJ_DIR = ./obj
SRCS = $(wildcard $(SRC_DIR)/*.c)

all: $(SRCS)
	$(CC) $(CFLAGS) -o backdoor_client $^ -I$(INC_DIR)
	@echo "Finished creating backdoor client."
