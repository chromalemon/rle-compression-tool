#include "unity.h"

// test_rle.c
void test_single_character_compression(void);
void test_single_character_decompression(void);
void test_no_repeat_compression(void);
void test_no_repeat_decompression(void);

// test_edge_cases.c
void test_run_of_255_fits_in_one_pair(void);
void test_run_of_256_splits_into_255_and_1(void);
void test_run_of_510_splits_into_two_full_runs(void);
void test_full_run_followed_by_different_byte(void);
void test_empty_input_compresses_to_nothing(void);
void test_empty_input_decompresses_to_nothing(void);
void test_multi_byte_blocks_compress(void);
void test_round_trip_all_block_sizes(void);
void test_compress_rejects_partial_final_block(void);
void test_compress_rejects_invalid_block_size(void);
void test_decompress_rejects_truncated_value(void);
void test_decompress_rejects_count_without_value(void);
void test_decompress_rejects_zero_count(void);
void test_decompress_stops_at_buffer_limit(void);

// test_files.c
void test_file_round_trip_empty(void);
void test_file_round_trip_size_not_multiple_of_block(void);
void test_file_round_trip_smaller_than_block(void);
void test_file_round_trip_many_sizes(void);
void test_decompress_rejects_block_size_zero(void);
void test_decompress_rejects_block_size_too_large(void);
void test_decompress_rejects_header_larger_than_data(void);
void test_decompress_rejects_huge_header_size(void);
void test_decompress_rejects_header_smaller_than_data(void);
void test_decompress_rejects_truncated_file(void);
void test_bmp_round_trip_all_padding_amounts(void);
void test_bmp_larger_image_round_trip(void);

int main(void) {
	UNITY_BEGIN();

	RUN_TEST(test_single_character_compression);
	RUN_TEST(test_single_character_decompression);
	RUN_TEST(test_no_repeat_compression);
	RUN_TEST(test_no_repeat_decompression);

	RUN_TEST(test_run_of_255_fits_in_one_pair);
	RUN_TEST(test_run_of_256_splits_into_255_and_1);
	RUN_TEST(test_run_of_510_splits_into_two_full_runs);
	RUN_TEST(test_full_run_followed_by_different_byte);
	RUN_TEST(test_empty_input_compresses_to_nothing);
	RUN_TEST(test_empty_input_decompresses_to_nothing);
	RUN_TEST(test_multi_byte_blocks_compress);
	RUN_TEST(test_round_trip_all_block_sizes);
	RUN_TEST(test_compress_rejects_partial_final_block);
	RUN_TEST(test_compress_rejects_invalid_block_size);
	RUN_TEST(test_decompress_rejects_truncated_value);
	RUN_TEST(test_decompress_rejects_count_without_value);
	RUN_TEST(test_decompress_rejects_zero_count);
	RUN_TEST(test_decompress_stops_at_buffer_limit);

	RUN_TEST(test_file_round_trip_empty);
	RUN_TEST(test_file_round_trip_size_not_multiple_of_block);
	RUN_TEST(test_file_round_trip_smaller_than_block);
	RUN_TEST(test_file_round_trip_many_sizes);
	RUN_TEST(test_decompress_rejects_block_size_zero);
	RUN_TEST(test_decompress_rejects_block_size_too_large);
	RUN_TEST(test_decompress_rejects_header_larger_than_data);
	RUN_TEST(test_decompress_rejects_huge_header_size);
	RUN_TEST(test_decompress_rejects_header_smaller_than_data);
	RUN_TEST(test_decompress_rejects_truncated_file);
	RUN_TEST(test_bmp_round_trip_all_padding_amounts);
	RUN_TEST(test_bmp_larger_image_round_trip);

	return UNITY_END();
}
