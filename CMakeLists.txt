cmake_minimum_required(VERSION 3.12)
project(rle-compression-tool LANGUAGES C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_compile_options(-Wall -Wextra -Werror)

set(CMAKE_C_FLAGS_DEBUG "-g -O0")
set(CMAKE_C_FLAGS_RELEASE "-O2 -DNDEBUG")

if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose build type" FORCE)
endif()

file(GLOB ALL_SRCS "src/*.c")

set(LIB_SRCS
	src/file.c
	src/bmp.c
	src/rle.c
)

file(GLOB ALL_HDRS "include/*.h")

add_executable(rle_tool ${ALL_SRCS})

target_include_directories(rle_tool PRIVATE include)

enable_testing()

set(TEST_SRCS
	tests/test_main.c
	tests/test_rle.c
	external/Unity/src/unity.c
)

add_executable(run_tests ${TEST_SRCS} ${LIB_SRCS})

target_include_directories(run_tests PRIVATE include external/Unity/src)
target_compile_options(run_tests PRIVATE -fsanitize=address -fsanitize=undefined -g)
target_link_options(run_tests PRIVATE -fsanitize=address -fsanitize=undefined)

add_test(NAME CoreTests COMMAND run_tests)

file(GLOB_RECURSE ALL_DEV_FILES "src/*.c" "tests/*.c" "include/*.h")

add_custom_target(format
	COMMAND clang-format -i ${ALL_DEV_FILES}
	COMMENT "Formatting source code with clang-format..."
)

add_custom_target(check-format
	COMMAND clang-format --dry-run -Werror ${ALL_DEV_FILES}
	COMMENT "Checking code compliance with clang-format..."
)

add_custom_target(lint
	COMMAND clang-tidy --warnings-as-errors='*' ${ALL_SRCS}
		-checks=-clang-analyzer-security.insecureAPI.DeprecatedOrUnsafeBufferHandling
		-- -I${CMAKE_CURRENT_SOURCE_DIR}/include ${CMAKE_C_FLAGS_${CMAKE_BUILD_TYPE}}
	COMMENT "Linting source files with clang-tidy"
)
