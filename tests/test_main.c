#include "unity.h"

void test_empty_string_compresses_to_empty(void);
void test_single_character(void);

int main(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_empty_string_compresses_to_empty);
    RUN_TEST(test_single_character);

    return UNITY_END();
}
