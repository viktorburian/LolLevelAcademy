# Makefile
CC = clang
CFLAGS = -Iinclude
DEPS = include/common.h include/file.h include/parse.h
OBJ = src/main.o src/file.o src/parse.o
TARGET = bin/database

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) -o $@ $^

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

clean:
	rm -f src/*.o $(TARGET)
