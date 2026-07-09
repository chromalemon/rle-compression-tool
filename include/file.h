#ifndef FILE_H
#define FILE_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#pragma pack(push, 1)
typedef struct {
	uint32_t signature;
	uint8_t block_size;
	size_t file_size;
} reg_meta;
#pragma pack(pop)

typedef struct {
	FILE *infile;
	FILE *outfile;
} file_struct;

typedef struct {
	uint8_t *inp_buf;
	size_t max_in_size;
	uint8_t *out_buf;
	uint8_t *out_ptr;
	size_t max_out_size;
	uint8_t block_size;
} mem_struct;

int file_mem_init(file_struct *file, mem_struct *mem, const char* input_path, const char* output_path, const uint8_t block_size, const size_t pixel_offset, const size_t size_offset);
long calc_file_size(const char* path);
void print_diff(const char* input_path, const char* output_path);


#endif
