#ifndef BMP_H
#define BMP_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define BMP_DEFAULTS .file_type = 0x4D42, .reserved1 = 0, .reserved2 = 0, .offset = 54, .DIB_size = 40, .planes = 1, .compression = 0, .x_res = 2835, .y_res = 2835, .colors_used = 0, .colors_important = 0

#pragma pack(push, 1)
typedef struct BMP_meta {
	uint16_t file_type;
	uint32_t file_size;
	uint16_t reserved1;
	uint16_t reserved2;
	uint32_t offset;
	uint32_t DIB_size;
	uint32_t width;
	uint32_t height;
	uint16_t planes;
	uint16_t BPP;
	uint32_t compression;
	uint32_t image_size;
	uint32_t x_res;
	uint32_t y_res;
	uint32_t colors_used;
	uint32_t colors_important;
} BMP_meta;
#pragma pack(pop)

int read_meta(FILE* infile, BMP_meta *meta);
void compress_bmp(const char* input_path, const char* output_path);
void decompress_bmp(const char* input_path, const char* output_path);

#endif
