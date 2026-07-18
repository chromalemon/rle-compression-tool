#ifndef BMP_H
#define BMP_H

#include <stdint.h>

#pragma pack(push, 1)

typedef struct {
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

int read_meta(const char *path, BMP_meta *meta);
int compress_bmp(const char *input_path, const char *output_path);
int decompress_bmp(const char *input_path, const char *output_path);

#endif
