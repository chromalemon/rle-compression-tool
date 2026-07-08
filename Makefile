CC = gcc
CFLAGS = -Wall -Wextra -Werror -std=gnu99 -Iinclude -O2

ifeq ($(DEBUG), 1)
	CFLAGS += -g -O0
else
	CFLAGS += -DNDEBUG
endif

TARGET = rle_tool
SRCS = src/file.c src/bmp.c src/rle.c src/main.c
OBJS = $(SRCS:.c=.o)

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $^ -o $(TARGET)

src/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@
	
clean:
	rm -f src/*.o $(TARGET)
