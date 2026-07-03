CC = gcc
CFLAGS = -Wall -Wextra -Werror -std=c99 -Iinclude

TARGET = rle_tool
SRCS = src/bmp.c src/rle.c src/main.c
OBJS = src/bmp.o src/rle.o src/main.o

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $^ -o $(TARGET)

$(OBJS): %.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@
	
clean:
	rm -f *.o $(TARGET)