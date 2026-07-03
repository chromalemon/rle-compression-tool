#ifndef BMP_H
#define BMP_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#pragma pack(push, 1)
typedef struct BMP_meta {
	uint16_t file_type = 0x4D42;
	uint32_t file_size;
	uint16_t reserved1 = 0;
	uint16_t reserved2 = 0;
	uint32_t offset = 54;
	uint32_t DIB_size = 40;
	uint32_t width;
	uint32_t height;
	uint16_t planes = 1;
	uint16_t BPP;
	uint32_t compression = 0;
	uint32_t image_size;
	uint32_t x_res = 2835;
	uint32_t y_res = 2835;
	uint32_t colors_used = 0;
	uint32_t colors_important = 0;
} BMP_meta;
#pragma pack(pop)

int read_meta(FILE* infile, BMP_meta *meta);
void compress_bmp(const char* input_path, const char* output_path);
void decompress_bmp(const char* input_path, const char* output_path);

#endif