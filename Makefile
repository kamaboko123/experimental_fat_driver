# Makefile

CC = gcc
CFLAGS = -Wall -g
INCLUDES = -I./include

SRC_DIR = src/entry
SRC = $(wildcard $(SRC_DIR)/*.c)

LIB_SRC_DIR = src/lib
LIB_SRC = $(wildcard $(LIB_SRC_DIR)/*.c)

LIB_OBJ_DIR = $(TARGET_DIR)/lib
LIB_OBJ = $(LIB_SRC:$(LIB_SRC_DIR)/%.c=$(LIB_OBJ_DIR)/%.o)

TARGET_DIR = bin
TARGETS = $(SRC:$(SRC_DIR)/%.c=$(TARGET_DIR)/%)


all: $(TARGETS)
	@echo $(TARGETS)

$(TARGET_DIR)/%: $(SRC_DIR)/%.c $(LIB_OBJ)
	mkdir -p $(TARGET_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $^

$(LIB_OBJ_DIR)/%.o: $(LIB_SRC_DIR)/%.c
	mkdir -p $(LIB_OBJ_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $^

clean:
	rm -rf $(TARGET_DIR)

