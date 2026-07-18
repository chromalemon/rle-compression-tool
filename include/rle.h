#ifndef RLE_H
#define RLE_H

#include "file.h"

int compress(mem_struct *mem);
int write_to_buf(mem_struct *mem, const uint8_t count,
				 const uint8_t *prev_block);
int decompress(mem_struct *mem);
int compress_regular(const char *input_path, const char *output_path,
					 const uint8_t block_size);
int decompress_regular(const char *input_path, const char *output_path);
#endif
