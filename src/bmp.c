#include "rle.h"
#include "bmp.h"
#include "file.h"

/*
int decompress_bmp(const char* input_path, const char* output_path){
	FILE* infile = NULL;
	FILE* outfile = NULL;
	uint8_t *inp_buf = NULL;
	uint8_t *out_buf = NULL;
	int res = 0;

	infile = fopen(input_path, "rb");
	if (!infile){
		fprintf(stderr, "Error: Could not open input file.\n");
		goto cleanup;
	}
	
	BMP_meta meta;	

	res = read_meta(infile, &meta);
	if (res != 1){
		goto cleanup;
	}

	size_t unpadded_row_length = (meta.width * meta.BPP) / 8;
	size_t total_padded_row_length = ((unpadded_row_length + 3) / 4) * 4;
	uint8_t padding_bytes_to_skip = total_padded_row_length - unpadded_row_length;
	size_t max_out_size = meta.height * unpadded_row_length;
	uint8_t block_size = meta.BPP / 8;
	uint8_t zero = 0;	

	outfile = fopen(output_path, "wb");
	if (!outfile){
		fprintf(stderr, "Error: Could not open output file.\n");
		goto cleanup;
	}

	if (!fwrite(&meta, sizeof(BMP_meta), 1, outfile)){
		fprintf(stderr, "Error: Could not write BMP metadata to file.\n");
		goto cleanup;
	}
	
	long curr_size = calc_file_size(input_path);
	size_t payload_size = curr_size - meta.offset;
	inp_buf = malloc(payload_size);

	if (inp_buf == NULL){
		fprintf(stderr, "Error: Could not allocate memory for input buffer.\n");
		goto cleanup;
	}

	if (fread(inp_buf, 1, payload_size, infile) < payload_size){
		fprintf(stderr, "Error: Could not read input file for payload.\n");
		goto cleanup;
	}	

	out_buf = malloc(max_out_size);
	if (out_buf == NULL){
		fprintf(stderr, "Error: Could not allocate memory for output buffer.\n");
		goto cleanup;
	}

	uint8_t *out_ptr = out_buf;

	res = decompress(inp_buf, payload_size, out_buf, &out_ptr, max_out_size, block_size);
	if (res != 1) goto cleanup;

	out_ptr = out_buf;
	for (uint8_t i = 0; i < meta.height; i++){
		if (fwrite(out_ptr, 1, unpadded_row_length, outfile) < unpadded_row_length){
			fprintf(stderr, "Error: Could not write payload to file.\n");
			goto cleanup;
		}
		out_ptr += unpadded_row_length;
		if (fwrite(&zero, 1, padding_bytes_to_skip, outfile) < padding_bytes_to_skip){
			fprintf(stderr, "Error: Could not write payload to file.\n");
			goto cleanup;
		}
		out_ptr += padding_bytes_to_skip;
	}	

	res = 1;	
	goto cleanup;
	
cleanup:
	if (infile) fclose(infile);
	if (outfile) fclose(outfile);
	free(inp_buf);
	free(out_buf);
	return (res == 1);
}
*/

int decompress_bmp(const char* input_path, const char* output_path){
	int res = 0;

	file_struct file;
	res = file_init(&file, input_path, output_path);
	if (res != 1) goto cleanup;
	
	BMP_meta meta;	

	res = read_meta(file.infile, &meta);
	if (res != 1){
		goto cleanup;
	}

	size_t unpadded_row_length = (meta.width * meta.BPP) / 8;
	size_t total_padded_row_length = ((unpadded_row_length + 3) / 4) * 4;
	uint8_t padding_bytes_to_skip = total_padded_row_length - unpadded_row_length;
	size_t max_out_size = meta.file_size - meta.offset;
	uint8_t block_size = meta.BPP / 8;
	uint8_t zero = 0;	

	mem_struct mem;
	res = mem_init(&mem, input_path, block_size, 0, max_out_size);
	mem.max_in_size -= meta.offset;

	if (!fwrite(&meta, sizeof(BMP_meta), 1, file.outfile)){
		fprintf(stderr, "Error: Could not write BMP metadata to file.\n");
		res = 0;
		goto cleanup;
	}

	if (!fread(mem.inp_buf, mem.max_in_size, 1, file.infile)){
		fprintf(stderr, "Error: Could not read file into input buffer.\n");
		res = 0;
		goto cleanup;
	}

	res = decompress(&mem);
	if (res != 1) goto cleanup;

	for (uint8_t i = 0; i < meta.height; i++){
		if (!fwrite(mem.out_ptr, unpadded_row_length, 1, file.outfile)){
			fprintf(stderr, "Error: Could not write payload to file.\n");
			res = 0;
			goto cleanup;
		}
		mem.out_ptr += unpadded_row_length;
		if (fwrite(&zero, 1, padding_bytes_to_skip, file.outfile) < padding_bytes_to_skip){
			fprintf(stderr, "Error: Could not write payload to file.\n");
			res = 0;
			goto cleanup;
		}
		mem.out_ptr += padding_bytes_to_skip;
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

/*
int compress_bmp(const char* input_path, const char* output_path){
	FILE* infile = NULL;
	FILE* outfile = NULL;
	uint8_t *inp_buf = NULL;
	uint8_t *out_buf = NULL;
	int res = 0;

	infile = fopen(input_path, "rb");
	if (!infile){
		fprintf(stderr, "Error: Could not open input file.\n");
		goto cleanup;
	}	

	BMP_meta meta;
	res = read_meta(infile, &meta);
	if (res != 1) goto cleanup;
	
	size_t unpadded_row_length = (meta.width * meta.BPP ) / 8;
	size_t total_padded_row_length = ((unpadded_row_length + 3) / 4) * 4;
	uint8_t padding_bytes_to_skip = total_padded_row_length - unpadded_row_length;
	size_t pure_pixel_bytes = meta.height * unpadded_row_length;
	size_t max_out_size = pure_pixel_bytes * 2;
	uint8_t block_size = meta.BPP / 8;

	inp_buf = malloc(pure_pixel_bytes);
	if (inp_buf == NULL){
		fprintf(stderr, "Error: Could not allocate memory for input buffer.\n");
		goto cleanup;
	} 
	uint8_t *inp_ptr = inp_buf;

	out_buf = malloc(max_out_size);
	if (out_buf == NULL){
		fprintf(stderr, "Error: Could not allocate memory for output buffer.\n");
		goto cleanup;
	}

	for (uint32_t i = 0; i < meta.height; i++){
		if (fread(inp_ptr, 1, unpadded_row_length, infile) < unpadded_row_length){
			fprintf(stderr, "Error: Could not read pixels from input file.\n");
			goto cleanup;
		}
		inp_ptr += unpadded_row_length;
		fseek(infile, padding_bytes_to_skip, SEEK_CUR);
	}


	uint8_t *out_ptr = out_buf;
	res = compress(inp_buf, pure_pixel_bytes, out_buf, &out_ptr, max_out_size, block_size);

	if (res != 1) goto cleanup;
	
	outfile = fopen(output_path, "wb");
	if (!outfile){
		fprintf(stderr, "Error: Could not open output file.\n");
		goto cleanup;
	}
	
	if (!fwrite(&meta, sizeof(BMP_meta), 1, outfile)){
		fprintf(stderr, "Error: Could not write BMP metadata to file.\n");
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
	if (inp_buf) free(inp_buf);
	if (out_buf) free(out_buf);
	if (infile) fclose(infile);
	if (outfile) fclose(outfile);
	return (res == 1);
}
*/

int compress_bmp(const char* input_path, const char* output_path){
	int res = 0;

	file_struct file;
	res = file_init(&file, input_path, output_path);
	if (res != 1) goto cleanup;
	
	BMP_meta meta;	

	res = read_meta(file.infile, &meta);
	if (res != 1){
		goto cleanup;
	}
	
	size_t unpadded_row_length = (meta.width * meta.BPP ) / 8;
	size_t total_padded_row_length = ((unpadded_row_length + 3) / 4) * 4;
	uint8_t padding_bytes_to_skip = total_padded_row_length - unpadded_row_length;
	size_t pure_pixel_bytes = meta.height * unpadded_row_length;
	size_t max_in_size = meta.file_size - meta.offset;
	size_t max_out_size = pure_pixel_bytes * 2;
	uint8_t block_size = meta.BPP / 8;

	mem_struct mem;
	res = mem_init(&mem, input_path, block_size, max_in_size, max_out_size);
	if (res != 1) goto cleanup;

	uint8_t *inp_ptr = mem.inp_buf;
	for (uint32_t i = 0; i < meta.height; i++){
		if (!fread(inp_ptr, unpadded_row_length, 1, file.infile)){
			fprintf(stderr, "Error: Could not read pixels from input file.\n");
			res = 0;
			goto cleanup;
		}
		inp_ptr += unpadded_row_length;
		fseek(file.infile, padding_bytes_to_skip, SEEK_CUR);
	}

	res = compress(&mem);
	if (res != 1) goto cleanup;
	
	if (!fwrite(&meta, sizeof(BMP_meta), 1, file.outfile)){
		fprintf(stderr, "Error: Could not write BMP metadata to file.\n");
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

int read_meta(FILE* infile, BMP_meta *meta){
	if (!meta) return 0;

	uint8_t sig_buf[2];
	if (fread(&sig_buf, 1, 2, infile) < 2){
		fprintf(stderr, "Error: Could not read file.\n");
		return 0;
	}

	if (memcmp("BM", sig_buf, 2) != 0){
		fprintf(stderr, "Error: Invalid signature, file must be BMP V3.\n");
		return 0;
	}
	
	fseek(infile, 0, SEEK_SET);
	if (!fread(meta, sizeof(BMP_meta), 1, infile)){
		fprintf(stderr, "Error: Could not parse BMP header.\n");
		return 0;
	}

	if (meta->DIB_size != 40 || meta->offset != 54 || meta->BPP != 24){
		fprintf(stderr, "Error: BMP file must be V3, 24 BPP.\n");
		return 0;
	}

	return 1;
}
