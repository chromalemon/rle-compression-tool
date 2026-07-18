#include "file.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

int file_init(file_struct *file, const char *input_path,
			  const char *output_path) {
	file->infile = NULL;
	file->outfile = NULL;
	file->input_size = 0;

	file->infile = fopen(input_path, "rb");
	if (!file->infile) {
		(void)fprintf(stderr, "Error: Could not open input file.\n");
		goto cleanup;
	}

	file->outfile = fopen(output_path, "wb");
	if (!file->outfile) {
		(void)fprintf(stderr, "Error: Could not open output file.\n");
		goto cleanup;
	}

	long res = calc_file_size(input_path);
	if (res == -1L) {
		goto cleanup;
	}
	size_t input_size = (size_t)res;
	file->input_size = input_size;

	return 1;

cleanup:
	if (file->infile) {
		(void)fclose(file->infile);
		file->infile = NULL;
	}
	if (file->outfile) {
		(void)fclose(file->outfile);
		file->outfile = NULL;
	}
	return 0;
}

int mem_init(mem_struct *mem, const uint8_t block_size,
			 const size_t pixel_offset, const size_t old_size,
			 const size_t input_size) {
	mem->inp_buf = NULL;
	mem->out_buf = NULL;

	mem->max_in_size = (size_t)(input_size - pixel_offset);

	mem->inp_buf = malloc(mem->max_in_size);
	if (mem->inp_buf == NULL) {
		(void)fprintf(stderr,
					  "Error: Could not allocate memory for input buffer.\n");
		goto cleanup;
	}

	if (old_size == 0) {
		mem->max_out_size = mem->max_in_size * 2;
	} else {
		mem->max_out_size = old_size;
	}

	mem->out_buf = malloc(mem->max_out_size);
	if (mem->out_buf == NULL) {
		(void)fprintf(stderr,
					  "Error: Could not allocate memory for output buffer.\n");
		goto cleanup;
	}

	mem->out_ptr = mem->out_buf;
	mem->block_size = block_size;

	return 1;

cleanup:
	if (mem->inp_buf) {
		free(mem->inp_buf);
		mem->inp_buf = NULL;
	}
	if (mem->out_buf) {
		free(mem->out_buf);
		mem->out_buf = NULL;
	}
	return 0;
}

long calc_file_size(const char *path) {
	FILE *fp = fopen(path, "rb");
	if (!fp) {
		(void)fprintf(stderr,
					  "Error: Could not open file to calculate size.\n");
		goto cleanup;
	}

	if (fseek(fp, 0L, SEEK_END) != 0) {
		(void)fprintf(stderr, "Error: Fseek failed.\n");
		goto cleanup;
	}
	long size = ftell(fp);
	if (size == -1L) {
		(void)fprintf(stderr, "Error: Ftell failed.\n");
		goto cleanup;
	}

	(void)fclose(fp);
	return size;

cleanup:
	if (fp) {
		(void)fclose(fp);
	}
	return -1L;
}

void print_diff(const char *input_path, const char *output_path) {
	long original = calc_file_size(input_path);
	long new = calc_file_size(output_path);
	if (original == -1L || new == -1L) {
		return;
	}

	double ratio = ((double)new / (double)original) * 100;

	printf("\nOriginal: %ld", original);
	printf("\nNew: %ld", new);
	printf("\nCompression ratio: %.2lf%%\n", ratio);
}
