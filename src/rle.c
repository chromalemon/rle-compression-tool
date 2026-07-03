#include "rle.h"

int compress(uint8_t *inp_buf, size_t inp_size, uint8_t *out_buf, uint8_t **out_ptr, const size_t max_out_size, const uint16_t block_size){
	uint8_t *curr_block = inp_buf;
	uint8_t count = 1;
	uint8_t *prev_block = inp_buf;
	curr_block += block_size;

	while (curr_block < (inp_buf + inp_size)){
		if (memcmp(prev_block, curr_block, block_size) == 0){
			count += 1;
			if (count == 255){
				if (write_to_buf(out_buf, out_ptr, block_size, max_out_size, count, prev_block) != 1){
					perror("Could not write to output buffer.\n");
					return 0;
				}
				count = 0;		
			}
		} else {
			if (count > 0){
				if (write_to_buf(out_buf, out_ptr, block_size, max_out_size, count, prev_block) != 1){
					perror("Could not write to output buffer.\n");
					return 0;
				}
			}
			prev_block = curr_block;
			count = 1;
		}
		curr_block += block_size;
	}
	if (count > 0 && write_to_buf(out_buf, out_ptr, block_size, max_out_size, count, prev_block) != 1){
		perror("Could not write to output buffer.\n");
		return 0;
	}

	printf("Compression complete.\n");
	return 1;
}

int write_to_buf(uint8_t *out_buf, uint8_t **out_ptr, const uint16_t block_size, const size_t max_out_size, const uint8_t count, const uint8_t *prev_block){
	size_t curr_size = *out_ptr - out_buf;
	if (curr_size + 1 + block_size > max_out_size){
		return 0;
	}
	
	**out_ptr = count;
	(*out_ptr)++;

	memcpy(*out_ptr, prev_block, block_size);
	(*out_ptr) += block_size;

	return 1;
}

int decompress(uint8_t *inp_buf, size_t inp_size, uint8_t *out_buf, uint8_t **out_ptr, const size_t max_out_size, const uint16_t block_size){
	uint8_t *count = inp_buf;
	uint8_t *curr_block = inp_buf + 1;
	
	while (curr_block < (inp_buf + inp_size)){
		for (uint8_t i = 0; i < *count; i++){
			size_t curr_size = *out_ptr - out_buf;
			if (curr_size + block_size > max_out_size){
				return 0;
			}
			
			memcpy(*out_ptr, curr_block, block_size);
			(*out_ptr) += block_size;	
		}
		count += block_size + 1;
		curr_block += block_size + 1;
	}

	return 1;
	
}

/*
void compress_file(const char* input_path, const char* output_path){
	FILE* infile = fopen(input_path, "rb");
	FILE* outfile = fopen(output_path, "wb");

	if (!infile || !outfile){
		perror("Could not open files.\n");
		if (infile) fclose(infile);
		if (outfile) fclose(outfile);
		return;
	}
	
	int curr_b;
	int prev_b = fgetc(infile);

	if (prev_b == EOF){
		fclose(infile);
		fclose(outfile);
		return;
	}
	
	unsigned char count = 1;

	while (curr_b != EOF){
		curr_b = fgetc(infile);
		if (curr_b == EOF) break;
		if (curr_b == prev_b){
			count += 1;
			if (count == 255){
				fputc(count, outfile);
				fputc(prev_b, outfile);
				count = 0;
			}
		}else{
			if (count == 0){
				prev_b = curr_b;
				count = 1;
				
			}else{
				fputc(count, outfile);
				fputc(prev_b, outfile);
				prev_b = curr_b;
				count = 1; 
			}
		}
	}
	fputc(count, outfile);
	fputc(prev_b, outfile);

	fclose(infile);
	fclose(outfile);
	printf("Compression complete.\n");
	return;	
}





void decompress_file(const char* input_path, const char* output_path){
	FILE* infile = fopen(input_path, "rb");
	FILE* outfile = fopen(output_path, "wb");

	if (!infile || !outfile){
		if (infile) fclose(infile);
		if (outfile) fclose(outfile);
		perror("Could not open files.\n");
		return;
	}
	
	int curr_b;
	int count = fgetc(infile);

	if (count == EOF){
		fclose(infile);
		fclose(outfile);
		return;
	}

	while (count != EOF){
		curr_b = fgetc(infile);
		for (int i = 0; i < count; i++){
			fputc(curr_b, outfile);
		}
		count = fgetc(infile);
	}

	fclose(infile);
	fclose(outfile);
	printf("Decompression complete.\n");
	return;
	
}
*/