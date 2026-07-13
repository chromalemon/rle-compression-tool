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

LIB_SRCS = src/file.c src/bmp.c src/rle.c

UNITY_DIR = external/Unity/src

TEST_SRCS = tests/test_main.c \
            tests_test_rle.c \
            $(LIB_SRCS) \
            $(UNITY_DIR)/unity.c

TEST_TARGET = run_tests

.PHONY: all clean test

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@

src/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@
	
test:

    $(CC) $(CFLAGS) \
    -I$(UNITY_DIR) \
    $(TEST_SRCS) \
    -o $(TEST_TARGET)
    ./$(TEST_TARGET)

clean:
	rm -f src/*.o $(TARGET) $(TEST_TARGET)
