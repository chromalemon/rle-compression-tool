#include "file.h"
#include "rle.h"
#include "unity.h"
#include <stdlib.h>
#include <string.h>

// Compresses `len` bytes of `input` and returns the compressed size, or -1
// if compression failed. The compressed bytes are copied into `out`.
static long compress_bytes(const uint8_t *input, size_t len, uint8_t block_size,
						   uint8_t *out) {
	mem_struct mem = {0};
	if (mem_init(&mem, block_size, 0, 0, len) != 1) {
		return -1;
	}
	if (len > 0) {
		memcpy(mem.inp_buf, input, len);
	}
	long res = -1;
	if (compress(&mem) == 1) {
		res = (long)(mem.out_ptr - mem.out_buf);
		memcpy(out, mem.out_buf, (size_t)res);
	}
	free(mem.inp_buf);
	free(mem.out_buf);
	return res;
}

// Decompresses `len` bytes into a buffer of `max_out` bytes and returns the
// decompressed size, or -1 if decompression failed.
static long decompress_bytes(const uint8_t *input, size_t len,
							 uint8_t block_size, size_t max_out, uint8_t *out) {
	mem_struct mem = {0};
	if (mem_init(&mem, block_size, 0, max_out, len) != 1) {
		return -1;
	}
	if (len > 0) {
		memcpy(mem.inp_buf, input, len);
	}
	long res = -1;
	if (decompress(&mem) == 1) {
		res = (long)(mem.out_ptr - mem.out_buf);
		memcpy(out, mem.out_buf, (size_t)res);
	}
	free(mem.inp_buf);
	free(mem.out_buf);
	return res;
}

void test_run_of_255_fits_in_one_pair(void) {
	uint8_t input[255];
	memset(input, 'A', sizeof(input));
	uint8_t out[16];

	long size = compress_bytes(input, sizeof(input), 1, out);

	uint8_t expected[] = {255, 'A'};
	TEST_ASSERT_EQUAL_INT(2, size);
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected, out, 2);
}

void test_run_of_256_splits_into_255_and_1(void) {
	uint8_t input[256];
	memset(input, 'A', sizeof(input));
	uint8_t out[16];

	long size = compress_bytes(input, sizeof(input), 1, out);

	uint8_t expected[] = {255, 'A', 1, 'A'};
	TEST_ASSERT_EQUAL_INT(4, size);
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected, out, 4);
}

void test_run_of_510_splits_into_two_full_runs(void) {
	uint8_t input[510];
	memset(input, 'A', sizeof(input));
	uint8_t out[16];

	long size = compress_bytes(input, sizeof(input), 1, out);

	uint8_t expected[] = {255, 'A', 255, 'A'};
	TEST_ASSERT_EQUAL_INT(4, size);
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected, out, 4);
}

void test_full_run_followed_by_different_byte(void) {
	uint8_t input[256];
	memset(input, 'A', 255);
	input[255] = 'B';
	uint8_t out[16];

	long size = compress_bytes(input, sizeof(input), 1, out);

	uint8_t expected[] = {255, 'A', 1, 'B'};
	TEST_ASSERT_EQUAL_INT(4, size);
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected, out, 4);
}

void test_empty_input_compresses_to_nothing(void) {
	uint8_t out[16];
	long size = compress_bytes(NULL, 0, 1, out);
	TEST_ASSERT_EQUAL_INT(0, size);
}

void test_empty_input_decompresses_to_nothing(void) {
	uint8_t out[16];
	long size = decompress_bytes(NULL, 0, 1, 0, out);
	TEST_ASSERT_EQUAL_INT(0, size);
}

void test_multi_byte_blocks_compress(void) {
	// three identical 3-byte pixels, then a different one
	uint8_t input[] = {1, 2, 3, 1, 2, 3, 1, 2, 3, 9, 9, 9};
	uint8_t out[16];

	long size = compress_bytes(input, sizeof(input), 3, out);

	uint8_t expected[] = {3, 1, 2, 3, 1, 9, 9, 9};
	TEST_ASSERT_EQUAL_INT(8, size);
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected, out, 8);
}

void test_round_trip_all_block_sizes(void) {
	// a mix of runs and noise, sized to be a multiple of every block size
	enum { LEN = 840 };
	uint8_t input[LEN];
	unsigned int seed = 12345;
	for (size_t i = 0; i < LEN; i++) {
		if (i < 300) {
			input[i] = 'A';
		} else {
			seed = seed * 1103515245U + 12345U;
			input[i] = (uint8_t)(seed >> 16);
		}
	}

	for (uint8_t block_size = 1; block_size <= 8; block_size++) {
		uint8_t compressed[LEN * 2];
		uint8_t restored[LEN];

		long c_size = compress_bytes(input, LEN, block_size, compressed);
		TEST_ASSERT_TRUE(c_size > 0);

		long d_size = decompress_bytes(compressed, (size_t)c_size, block_size,
									   LEN, restored);
		TEST_ASSERT_EQUAL_INT(LEN, d_size);
		TEST_ASSERT_EQUAL_UINT8_ARRAY(input, restored, LEN);
	}
}

void test_compress_rejects_partial_final_block(void) {
	uint8_t input[10] = {0};
	uint8_t out[32];
	TEST_ASSERT_EQUAL_INT(-1, compress_bytes(input, sizeof(input), 3, out));
}

void test_compress_rejects_invalid_block_size(void) {
	uint8_t input[9] = {0};
	uint8_t out[32];
	TEST_ASSERT_EQUAL_INT(-1, compress_bytes(input, sizeof(input), 0, out));
	TEST_ASSERT_EQUAL_INT(-1, compress_bytes(input, sizeof(input), 9, out));
}

void test_decompress_rejects_truncated_value(void) {
	// count 2, then only 2 of the 4 bytes of the value
	uint8_t input[] = {2, 'A', 'A'};
	uint8_t out[32];
	TEST_ASSERT_EQUAL_INT(-1,
						  decompress_bytes(input, sizeof(input), 4, 8, out));
}

void test_decompress_rejects_count_without_value(void) {
	uint8_t input[] = {2, 'A', 3};
	uint8_t out[32];
	TEST_ASSERT_EQUAL_INT(-1,
						  decompress_bytes(input, sizeof(input), 1, 8, out));
}

void test_decompress_rejects_zero_count(void) {
	uint8_t input[] = {0, 'A'};
	uint8_t out[32];
	TEST_ASSERT_EQUAL_INT(-1,
						  decompress_bytes(input, sizeof(input), 1, 8, out));
}

void test_decompress_stops_at_buffer_limit(void) {
	// expands to 255 bytes, but the output buffer only holds 10
	uint8_t input[] = {255, 'A'};
	uint8_t out[256];
	TEST_ASSERT_EQUAL_INT(-1,
						  decompress_bytes(input, sizeof(input), 1, 10, out));
}
