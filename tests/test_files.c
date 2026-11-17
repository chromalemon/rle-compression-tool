#include "bmp.h"
#include "file.h"
#include "rle.h"
#include "unity.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ORIGINAL_PATH "test_original.tmp"
#define COMPRESSED_PATH "test_compressed.tmp"
#define RESTORED_PATH "test_restored.tmp"

static void write_file(const char *path, const uint8_t *data, size_t len) {
	FILE *fp = fopen(path, "wb");
	TEST_ASSERT_NOT_NULL(fp);
	if (len > 0) {
		TEST_ASSERT_EQUAL_size_t(len, fwrite(data, 1, len, fp));
	}
	(void)fclose(fp);
}

// Reads a whole file into a malloc'd buffer and stores its size in *len.
static uint8_t *read_file(const char *path, size_t *len) {
	FILE *fp = fopen(path, "rb");
	TEST_ASSERT_NOT_NULL(fp);
	(void)fseek(fp, 0, SEEK_END);
	long size = ftell(fp);
	(void)fseek(fp, 0, SEEK_SET);
	uint8_t *buf = malloc(size > 0 ? (size_t)size : 1);
	TEST_ASSERT_NOT_NULL(buf);
	*len = fread(buf, 1, (size_t)size, fp);
	(void)fclose(fp);
	return buf;
}

static void remove_temp_files(void) {
	(void)remove(ORIGINAL_PATH);
	(void)remove(COMPRESSED_PATH);
	(void)remove(RESTORED_PATH);
}

static void assert_regular_round_trip(const uint8_t *data, size_t len,
									  uint8_t block_size) {
	write_file(ORIGINAL_PATH, data, len);
	TEST_ASSERT_EQUAL_INT(
		1, compress_regular(ORIGINAL_PATH, COMPRESSED_PATH, block_size));
	TEST_ASSERT_EQUAL_INT(1,
						  decompress_regular(COMPRESSED_PATH, RESTORED_PATH));

	size_t restored_len = 0;
	uint8_t *restored = read_file(RESTORED_PATH, &restored_len);
	TEST_ASSERT_EQUAL_size_t(len, restored_len);
	if (len > 0) {
		TEST_ASSERT_EQUAL_UINT8_ARRAY(data, restored, len);
	}
	free(restored);
	remove_temp_files();
}

// Writes a compressed file with a hand-made header, to simulate a corrupted
// or malicious input.
static void write_crafted_rle(uint8_t block_size, size_t old_size,
							  const uint8_t *payload, size_t payload_len) {
	reg_meta meta = {0};
	memcpy(&meta.signature, "RLE!", 4);
	meta.block_size = block_size;
	meta.old_size = old_size;

	FILE *fp = fopen(COMPRESSED_PATH, "wb");
	TEST_ASSERT_NOT_NULL(fp);
	(void)fwrite(&meta, sizeof(meta), 1, fp);
	if (payload_len > 0) {
		(void)fwrite(payload, 1, payload_len, fp);
	}
	(void)fclose(fp);
}

void test_file_round_trip_empty(void) { assert_regular_round_trip(NULL, 0, 1); }

void test_file_round_trip_size_not_multiple_of_block(void) {
	uint8_t data[] = "AAAAAAAAAB"; // 10 bytes, word size 3
	assert_regular_round_trip(data, 10, 3);
}

void test_file_round_trip_smaller_than_block(void) {
	uint8_t data[] = {'A', 'B'};
	assert_regular_round_trip(data, sizeof(data), 8);
}

void test_file_round_trip_many_sizes(void) {
	uint8_t data[1001];
	for (size_t i = 0; i < sizeof(data); i++) {
		data[i] = (uint8_t)((i / 7) % 5); // runs of 7 bytes
	}
	size_t sizes[] = {1, 2, 3, 7, 255, 256, 1000, 1001};
	for (size_t s = 0; s < sizeof(sizes) / sizeof(sizes[0]); s++) {
		for (uint8_t block_size = 1; block_size <= 8; block_size++) {
			assert_regular_round_trip(data, sizes[s], block_size);
		}
	}
}

void test_decompress_rejects_block_size_zero(void) {
	uint8_t payload[] = {2, 'A'};
	write_crafted_rle(0, 2, payload, sizeof(payload));
	TEST_ASSERT_EQUAL_INT(0,
						  decompress_regular(COMPRESSED_PATH, RESTORED_PATH));
	remove_temp_files();
}

void test_decompress_rejects_block_size_too_large(void) {
	uint8_t payload[10] = {1};
	write_crafted_rle(9, 9, payload, sizeof(payload));
	TEST_ASSERT_EQUAL_INT(0,
						  decompress_regular(COMPRESSED_PATH, RESTORED_PATH));
	remove_temp_files();
}

void test_decompress_rejects_header_larger_than_data(void) {
	// header claims 100 bytes, but the data only expands to 2; writing it
	// out would leak 98 bytes of uninitialised heap memory
	uint8_t payload[] = {2, 'A'};
	write_crafted_rle(1, 100, payload, sizeof(payload));
	TEST_ASSERT_EQUAL_INT(0,
						  decompress_regular(COMPRESSED_PATH, RESTORED_PATH));
	remove_temp_files();
}

void test_decompress_rejects_huge_header_size(void) {
	uint8_t payload[] = {2, 'A'};
	write_crafted_rle(1, (size_t)-1, payload, sizeof(payload));
	TEST_ASSERT_EQUAL_INT(0,
						  decompress_regular(COMPRESSED_PATH, RESTORED_PATH));
	remove_temp_files();
}

void test_decompress_rejects_header_smaller_than_data(void) {
	uint8_t payload[] = {200, 'A'};
	write_crafted_rle(1, 5, payload, sizeof(payload));
	TEST_ASSERT_EQUAL_INT(0,
						  decompress_regular(COMPRESSED_PATH, RESTORED_PATH));
	remove_temp_files();
}

void test_decompress_rejects_truncated_file(void) {
	// shorter than the header itself
	uint8_t data[] = {'R', 'L', 'E'};
	write_file(COMPRESSED_PATH, data, sizeof(data));
	TEST_ASSERT_EQUAL_INT(0,
						  decompress_regular(COMPRESSED_PATH, RESTORED_PATH));
	remove_temp_files();
}

// Writes a 24-bit BMP3 whose pixels are a simple pattern, padding each row
// to a multiple of 4 bytes with zeros.
static void write_test_bmp(const char *path, uint32_t width, uint32_t height) {
	size_t row = (size_t)width * 3;
	size_t padded_row = ((row + 3) / 4) * 4;
	size_t pixel_size = padded_row * height;

	BMP_meta meta = {0};
	memcpy(&meta.file_type, "BM", 2);
	meta.file_size = (uint32_t)(sizeof(BMP_meta) + pixel_size);
	meta.offset = sizeof(BMP_meta);
	meta.DIB_size = 40;
	meta.width = width;
	meta.height = height;
	meta.planes = 1;
	meta.BPP = 24;
	meta.image_size = (uint32_t)pixel_size;

	uint8_t *pixels = calloc(pixel_size > 0 ? pixel_size : 1, 1);
	TEST_ASSERT_NOT_NULL(pixels);
	for (uint32_t y = 0; y < height; y++) {
		for (size_t x = 0; x < row; x++) {
			// left half one colour, right half another, so rows contain runs
			pixels[(y * padded_row) + x] = (x < row / 2) ? 0x10 : (uint8_t)y;
		}
	}

	FILE *fp = fopen(path, "wb");
	TEST_ASSERT_NOT_NULL(fp);
	(void)fwrite(&meta, sizeof(meta), 1, fp);
	(void)fwrite(pixels, 1, pixel_size, fp);
	(void)fclose(fp);
	free(pixels);
}

void test_bmp_round_trip_all_padding_amounts(void) {
	// widths 1-4 give 3, 2, 1 and 0 bytes of padding per row
	for (uint32_t width = 1; width <= 4; width++) {
		write_test_bmp(ORIGINAL_PATH, width, 5);
		TEST_ASSERT_EQUAL_INT(1, compress_bmp(ORIGINAL_PATH, COMPRESSED_PATH));
		TEST_ASSERT_EQUAL_INT(1,
							  decompress_bmp(COMPRESSED_PATH, RESTORED_PATH));

		size_t original_len = 0;
		size_t restored_len = 0;
		uint8_t *original = read_file(ORIGINAL_PATH, &original_len);
		uint8_t *restored = read_file(RESTORED_PATH, &restored_len);

		// byte-for-byte identical, including the file_size header field
		TEST_ASSERT_EQUAL_size_t(original_len, restored_len);
		TEST_ASSERT_EQUAL_UINT8_ARRAY(original, restored, original_len);

		free(original);
		free(restored);
		remove_temp_files();
	}
}

void test_bmp_larger_image_round_trip(void) {
	write_test_bmp(ORIGINAL_PATH, 101, 37);
	TEST_ASSERT_EQUAL_INT(1, compress_bmp(ORIGINAL_PATH, COMPRESSED_PATH));
	TEST_ASSERT_EQUAL_INT(1, decompress_bmp(COMPRESSED_PATH, RESTORED_PATH));

	size_t original_len = 0;
	size_t restored_len = 0;
	uint8_t *original = read_file(ORIGINAL_PATH, &original_len);
	uint8_t *restored = read_file(RESTORED_PATH, &restored_len);
	TEST_ASSERT_EQUAL_size_t(original_len, restored_len);
	TEST_ASSERT_EQUAL_UINT8_ARRAY(original, restored, original_len);

	free(original);
	free(restored);
	remove_temp_files();
}
