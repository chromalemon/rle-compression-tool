CC = gcc
CFLAGS = -Wall -Wextra -Werror -std=gnu99 -Iinclude -O2
SANITIZE_FLAGS = -fsanitize=address -fsanitize=undefined -g

ifeq ($(DEBUG), 1)
	CFLAGS += -g -O0
else
	CFLAGS += -DNDEBUG
endif

TARGET = rle_tool

SRCS = $(wildcard src/*.c)
HDRS = $(wildcard include/*.h)
OBJS = $(SRCS:.c=.o)

LIB_SRCS = src/file.c src/bmp.c src/rle.c

UNITY_DIR = external/Unity/src

TEST_SRCS = tests/test_main.c \
	        tests/test_rle.c \

TEST_TARGET = run_tests

.PHONY: all clean test format check-format lint

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@

src/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@
	
test:
	$(CC) $(CFLAGS) $(SANITIZE_FLAGS) \
	-I$(UNITY_DIR) \
	$(TEST_SRCS) \
	$(LIB_SRCS) \
	$(UNITY_DIR)/unity.c \
	-o $(TEST_TARGET)
	./$(TEST_TARGET)

format:
	clang-format -i $(SRCS) $(TEST_SRCS) $(HDRS)

check-format:
	clang-format --dry-run --Werror $(SRCS) $(TEST_SRCS) $(HDRS)

lint:
	clang-tidy $(SRCS) -checks=-clang-analyzer-security.insecureAPI.DeprecatedOrUnsafeBufferHandling -- $(CFLAGS)

clean:
	rm -f src/*.o $(TARGET) $(TEST_TARGET)
