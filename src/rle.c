#include "rle.h"
#include "file.h"

int compress(mem_struct *mem){
	uint8_t *curr_block = mem->inp_buf;
	uint8_t count = 1;
	uint8_t *prev_block = mem->inp_buf;
	curr_block += mem->block_size;

	while (curr_block < (mem->inp_buf + mem->max_in_size)){
		if (memcmp(prev_block, curr_block, mem->block_size) == 0){
			count += 1;
			if (count == 255){
				if (write_to_buf(mem, count, prev_block) != 1){
					fprintf(stderr, "Error: Could not write to output buffer.\n");
					return 0;
				}
				count = 0;		
			}
		} else {
			if (count > 0){
				if (write_to_buf(mem, count, prev_block) != 1){
					fprintf(stderr, "Error: Could not write to output buffer.\n");
					return 0;
				}
			}
			prev_block = curr_block;
			count = 1;
		}
		curr_block += mem->block_size;
	}
	if (count > 0 && !write_to_buf(mem, count, prev_block)){
		fprintf(stderr, "Error: Could not write to output buffer.\n");
		return 0;
	}

	printf("Compression complete.\n");
	return 1;
}

int write_to_buf(mem_struct *mem, const uint8_t count, const uint8_t *prev_block){
	size_t curr_size = mem->out_ptr - mem->out_buf;
	if (curr_size + 1 + mem->block_size > mem->max_out_size){
		fprintf(stderr, "Error: Output exceeded buffer size.\n");
		return 0;
	}
	
	*(mem->out_ptr) = count;
	(mem->out_ptr)++;

	memcpy(mem->out_ptr, prev_block, mem->block_size);
	mem->out_ptr += mem->block_size;

	return 1;
}

int decompress(mem_struct *mem){
	uint8_t *count = mem->inp_buf;
	uint8_t *curr_block = mem->inp_buf + 1;
	
	while (curr_block < (mem->inp_buf + mem->max_in_size)){
		for (uint8_t i = 0; i < *count; i++){
			size_t curr_size = mem->out_ptr - mem->out_buf;
			if (curr_size + mem->block_size > mem->max_out_size){
				fprintf(stderr, "Error: Output exceeded buffer size.\n");
				return 0;
			}
			
			memcpy(mem->out_ptr, curr_block, mem->block_size);
			mem->out_ptr += mem->block_size;	
		}
		count += mem->block_size + 1;
		curr_block += mem->block_size + 1;
	}

	printf("Decompression complete.\n");
	return 1;
	
}
/*
int compress_regular(const char *input_path, const char* output_path, const uint8_t block_size){
	FILE *infile = NULL;
	FILE *outfile = NULL;
	uint8_t *inp_buf = NULL;
	uint8_t *out_buf = NULL;
	int res = 0;

	infile = fopen(input_path, "rb");
	
	if (!infile){
		fprintf(stderr, "Error: Could not open input file.\n");
		goto cleanup;
	}

	long curr_size = calc_file_size(input_path);
	inp_buf = malloc(curr_size);

	if (inp_buf == NULL){
		fprintf(stderr, "Error: Could not allocate memory for input buffer.\n");
		goto cleanup;
	}

	if (!fread(inp_buf, curr_size, 1, infile)){
		fprintf(stderr, "Error: Could not read file into input buffer.\n");
		goto cleanup;
	}

	size_t max_out_size = curr_size * 2;
	out_buf = malloc(max_out_size);

	if (out_buf == NULL){
		fprintf(stderr, "Error: Could not open output file.\n");
		goto cleanup;
	}
	
	uint8_t *out_ptr = out_buf;
	res = compress(inp_buf, curr_size, out_buf, &out_ptr, max_out_size, block_size);
	if (res != 1) goto cleanup;

	outfile = fopen(output_path, "wb");
	if (!outfile){
		fprintf(stderr, "Error: Could not open output file.\n");
		goto cleanup;
	}
	
	size_t compressed_size = out_ptr - out_buf;
	if (fwrite(out_buf, 1, compressed_size, outfile) != compressed_size){
		fprintf(stderr, "Error: Could not write payload to file.\n");
		goto cleanup;
	}

	res = 1;
	goto cleanup;

cleanup:
	if (infile) fclose(infile);
	if (outfile) fclose(outfile);
	free(inp_buf);
	free(out_buf);
	return res;
}
*/

int compress_regular(const char* input_path, const char* output_path, const uint8_t block_size){
	int res = 0;
	
	file_struct file;
	res = file_init(&file, input_path, output_path);
	if (res != 1) goto cleanup; 

	mem_struct mem;
	res = mem_init(&mem, input_path, output_path, block_size);
	if (res != 1) goto cleanup;

	if (!fread(mem.inp_buf, mem.max_in_size, 1, file.infile)){
		fprintf(stderr, "Error: Could not read file into input buffer.\n");
		res = 0;
		goto cleanup;
	}

	res = compress(&mem);
	if (res != 1) goto cleanup;

	if (!fwrite("RLE!", 4, 1, mem.outfile) || !fwrite(&(mem.block_size), 1, 1, file.outfile) || !fwrite(&(mem.max_in_size), sizeof(size_t), 1 file.outfile)){
		fprintf(stderr, "Error: Could not write metadata to output file.\n");
		res = 0;
		goto cleanup;
	}
	
	size_t new_size = mem.out_ptr - mem.out_buf;
	if (!fwrite(mem.out_buf, new_size, 1, file.outfile)){
		fprintf(stderr, "Error: Could not write payload to file.\n");
		res = 0;
		goto cleanup;
	}
	
	res = 1;
	goto cleanup;

cleanup:
	if (file.infile) fclose(file.infile);
	if (file.outfile) fclose(file.outfile);
	free(mem.inp_buf);
	free(mem.out_buf);
	return res;
}

int decompress_regular(const char *input_path, const char* output_path){
	int res = 0;
	size_t max_out_size;
	uint8_t block_size;
	uint8_t sig_buf[4];

	file_struct file;
	res = file_init(&file, input_path, output_path);
	if (res != 1) goto cleanup;

	if (!fread(sig_buf, 4, 1, file.infile)){
		fprintf(stderr, "Error: Could not read file signature.\n");
		res = 0;
		goto cleanup;
	}
	if (memcmp("RLE!", sig_buf, 4) != 0){
		fprintf(stderr, "Error: Invalid file signature!\n");
		res = 0;
		goto cleanup;
	}

	if (!fread(&block_size, 1, 1, file.infile) !! !fread(&max_out_size, sizeof(size_t), 1, file.infile)){
		fprintf(stderr, "Error: Could not read metadata from input file.\n");
		res = 0;
		goto cleanup;
	}

	mem_struct mem;
	res = mem_init(&mem, input_path, output_path, block_size);
	if (res != 1) goto cleanup;
	
	mem.max_out_size = max_out_size;

	if (!fread(mem.inp_buf, mem.max_in_size, 1, file.infile)){
		fprintf(stderr, "Error: Could not read file into input buffer.\n");
		res = 0;
		goto cleanup;
	}

	res = decompress(&mem);
	if (res != 1) goto cleanup;

	size_t new_size = mem.out_ptr - mem.out_buf;
	if (!fwrite(mem.out_buf, new_size, 1, file.outfile)){
		fprintf(stderr, "Error: Could not write payload to file.\n");
		res = 0;
		goto cleanup;
	}
	
	res = 1;
	goto cleanup;

cleanup:
	if (file.infile) fclose(file.infile);
	if (file.outfile) fclose(file.outfile);
	free(mem.inp_buf);
	free(mem.out_buf);
	return res;

}
