#include "file.h"
#include "rle.h"
#include "unity.h"

void setUp(void) {
	// Runs before every test
}

void tearDown(void) {
	// Runs after every test
}

void test_single_character_compression(void) {
	mem_struct mem;
	int res = mem_init(&mem, 1, 0, 1, 0, 1);
	TEST_ASSERT_EQUAL_INT(1, res);

	unsigned char str[1] = {'A'};
	memcpy(mem.inp_buf, str, 1);

	res = compress(&mem);
	TEST_ASSERT_EQUAL_INT(1, res);

	unsigned char expected[2] = {1, 'A'};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected, mem.out_buf, 2);

	goto cleanup;

cleanup:
	free(mem.inp_buf);
	free(mem.out_buf);
}

void test_single_character_decompression(void) {
	mem_struct mem;
	int res = mem_init(&mem, 1, 0, 0, 1, 2);
	TEST_ASSERT_EQUAL_INT(1, res);

	unsigned char str[2] = {1, 'A'};
	memcpy(mem.inp_buf, str, 2);

	res = decompress(&mem);
	TEST_ASSERT_EQUAL_INT(1, res);

	TEST_ASSERT_EQUAL_UINT8('A', mem.out_buf[0]);

	goto cleanup;

cleanup:
	free(mem.inp_buf);
	free(mem.out_buf);
}

void test_no_repeat_compression(void) {
	mem_struct mem;
	int res = mem_init(&mem, 1, 0, 1, 0, 6);
	TEST_ASSERT_EQUAL_INT(1, res);

	unsigned char str[6] = {'A', 'B', 'C', 'D', 'E', 'F'};
	memcpy(mem.inp_buf, str, 6);

	res = compress(&mem);
	TEST_ASSERT_EQUAL_INT(1, res);

	unsigned char expected[12] = {1, 'A', 1, 'B', 1, 'C',
								  1, 'D', 1, 'E', 1, 'F'};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected, mem.out_buf, 12);

	goto cleanup;

cleanup:
	free(mem.inp_buf);
	free(mem.out_buf);
}

void test_no_repeat_decompression(void) {
	mem_struct mem;
	int res = mem_init(&mem, 1, 0, 0, 6, 12);
	TEST_ASSERT_EQUAL_INT(1, res);

	unsigned char str[12] = {1, 'A', 1, 'B', 1, 'C', 1, 'D', 1, 'E', 1, 'F'};
	memcpy(mem.inp_buf, str, 12);

	res = decompress(&mem);
	TEST_ASSERT_EQUAL_INT(1, res);

	unsigned char expected[6] = {'A', 'B', 'C', 'D', 'E', 'F'};
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected, mem.out_buf, 6);

	goto cleanup;

cleanup:
	free(mem.inp_buf);
	free(mem.out_buf);
}
