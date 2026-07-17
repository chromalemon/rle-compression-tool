#include "unity.h"

void test_single_character_compression(void);
void test_single_character_decompression(void);
void test_no_repeat_compression(void);
void test_no_repeat_decompression(void);

int main(void) {
	UNITY_BEGIN();

	RUN_TEST(test_single_character_compression);
	RUN_TEST(test_single_character_decompression);
	RUN_TEST(test_no_repeat_compression);
	RUN_TEST(test_no_repeat_decompression);

	return UNITY_END();
}
