#include "file.h"

int file_mem_init(file_struct *file, mem_struct *mem, const char* input_path, const char* output_path, const uint8_t block_size, const size_t pixel_offset, const size_t size_offset){
	FILE *infile = NULL;
	FILE *outfile = NULL;
	uint8_t *inp_buf = NULL;
	uint8_t *out_buf = NULL;

	infile = fopen(input_path, "rb");
	if (!infile){
		fprintf(stderr, "Error: Could not open input file.\n");
		goto cleanup;
	}

	outfile = fopen(output_path, "wb");
	if (!outfile){
		fprintf(stderr, "Error: Could not open output file.\n");
		goto cleanup;
	}

	file->infile = infile;
	file->outfile = outfile;
	
	long res = calc_file_size(input_path);
	if (res == -1L){
		goto cleanup;
	}
	
	size_t max_in_size = (size_t)(res - pixel_offset);

	inp_buf = malloc(max_in_size);
	if (inp_buf == NULL){
		fprintf(stderr, "Error: Could not allocate memory for input buffer.\n");
		goto cleanup;
	}

	size_t max_out_size;
	if (size_offset == 0){
		max_out_size = max_in_size * 2;
	} else {
		fseek(file->infile, (long)size_offset, SEEK_SET);
		if (!fread(&max_out_size, sizeof(size_t), 1, file->infile)){
			fprintf(stderr, "Error: Could not read input file metadata.\n");
			goto cleanup;
		}
		fseek(file->infile, 0, SEEK_SET);
	}

	out_buf = malloc(max_out_size);
	if (out_buf == NULL){
		fprintf(stderr, "Error: Could not allocate memory for output buffer.\n");
		goto cleanup;
	}

	mem->inp_buf = inp_buf;
	mem->max_in_size = max_in_size;
	mem->out_buf = out_buf;
	mem->out_ptr = out_buf;
	mem->max_out_size = max_out_size;
	mem->block_size = block_size;

	return 1;

cleanup:
	if (infile) fclose(infile);
	if (outfile) fclose(outfile);
	free(inp_buf);
	free(out_buf);
	return 0;
}

long calc_file_size(const char* path){
	FILE* fp = fopen(path, "rb");
	if (!fp){
		fprintf(stderr, "Error: Could not open file to calculate size.\n");
		return -1L;
	}
	
	if (fseek(fp, 0L, SEEK_END) != 0){
		fprintf(stderr, "Error: Fseek failed.\n");
		fclose(fp);
		return -1L;
	}
	long size = ftell(fp);
	if (size == -1L){
		fprintf(stderr, "Error: Ftell failed.\n");
	}

	fclose(fp);
	return size;

}

void print_diff(const char* input_path, const char* output_path){
	long original = calc_file_size(input_path);
	long new = calc_file_size(output_path);
	if (original == -1L || new == -1L){
		return;
	}

	float ratio = ((float)new / original) * 100;

	printf("\nOriginal: %ld", original);
	printf("\nNew: %ld", new);
	printf("\nCompression ratio: %.2f%%\n", ratio);
}
