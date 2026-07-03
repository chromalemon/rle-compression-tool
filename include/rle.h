#ifndef RLE_H
#define RLE_H

#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>

/*
void compress_file(const char* input_path, const char* output_path);
void decompress_file(const char* input_path, const char* output_path);
*/

int compress(uint8_t *inp_buf, size_t inp_size, uint8_t *out_buf, uint8_t **out_ptr, const size_t max_out_size, const uint16_t block_size);
int write_to_buf(uint8_t *out_buf, uint8_t **out_ptr, const uint16_t block_size, const size_t max_out_size, const uint8_t count, const uint8_t *prev_block);


#endif