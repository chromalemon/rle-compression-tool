#include "rle.h"
#include "bmp.h"
#include "calc.h"

void decompress_bmp(const char* input_path, const char* output_path){
	FILE* infile = fopen(input_path, "rb");
	FILE* outfile = fopen(output_path, "rw");
	uint8_t *inp_buf = NULL;
	uint8_t *out_buf = NULL;

	if (!infile || !outfile){
		perror("Could not open file.\n");
		goto cleanup;
	}
	
	BMP_meta meta;	

	if (!fread(&(meta.width), 4, 1, infile) || !fread(&(meta.height), 4, 1 , infile) || !fread(&(meta.BPP), 2, 1, infile)){
		perror("Could not read file.\n");
		goto cleanup;
	}
	
	size_t unpadded_row_length = (meta.width * meta.BPP) / 8;
	size_t total_padded_row_length = ((unpadded_row_length + 3) / 4) * 4;
	uint8_t padding_bytes_to_skip = total_padded_row_length - unpadded_row_length;
	uint32_t image_size = total_padded_row_length * height;
	size_t max_out_size = meta.height * unpadded_row_length;
	uint16_t block_size = meta.BPP / 8;

	meta.image_size = image_size;
	meta.file_size = image_size + 54;

	if (!fwrite(%meta, sizeof(BMP_meta), 1, outfile)){
		perror("Could not write BMP header.\n");
		goto cleanup;
	}

	long curr_file_size = calc_file_size(infile) - 54;
	inp_buf = malloc(curr_file_size);

	if (inp_buf == NULL){
		perror("Could not allocate memory.\n");
		goto cleanup;
	}

	

	goto cleanup;
	
cleanup:
	if (infile) fclose(infile);
	if (outfile) fclose(outfile);
	free(inp_buf);
	free(out_buf);
	return;
}

void compress_bmp(const char* input_path, const char* output_path){
	FILE* infile = NULL;
	FILE* outfile = NULL;
	uint8_t *inp_buf = NULL;
	uint8_t *out_buf = NULL;

	infile = fopen(input_path, "rb");
	if (!infile){
		perror("Could not open file.\n");
		return;
	}	

	BMP_meta meta;
	int res = read_meta(infile, &meta);
	if (res == 0){
		perror("Could not read file meta.\n");
		goto cleanup;
	}

	size_t unpadded_row_length = (meta.width * meta.BPP ) / 8;
	size_t total_padded_row_length = ((unpadded_row_length + 3) / 4) * 4;
	uint8_t padding_bytes_to_skip = total_padded_row_length - unpadded_row_length;
	size_t pure_pixel_bytes = meta.height * unpadded_row_length;
	size_t max_out_size = pure_pixel_bytes * 2;
	uint16_t block_size = meta.BPP / 8;

	inp_buf = malloc(pure_pixel_bytes);
	if (inp_buf == NULL){
		perror("Could not allocate memory.\n");
		goto cleanup;
	} 
	uint8_t *inp_ptr = inp_buf;

	out_buf = malloc(max_out_size);
	if (out_buf == NULL){
		perror("Could not allocate memory.\n");
		goto cleanup;
	}

	fseek(infile, meta.offset, SEEK_SET);
	
	for (uint32_t i = 0; i < meta.height; i++){
		if (fread(inp_ptr, 1, unpadded_row_length, infile) == 0){
			perror("Could not read file.\n");
			goto cleanup;
		}
		inp_ptr += unpadded_row_length;
		fseek(infile, padding_bytes_to_skip, SEEK_CUR);
	}


	uint8_t *out_ptr = out_buf;
	res = compress(inp_buf, pure_pixel_bytes, out_buf, &out_ptr, max_out_size, block_size);
	if (res != 1){
		perror("Could not compress BMP file.\n");
		goto cleanup;
	}
	
	size_t compressed_size = out_ptr - out_buf;

	outfile = fopen(output_path, "wb");
	if (!outfile){
		perror("Could not open file.\n");
		goto cleanup;
	}
	
	if (!fwrite(&(meta.width), 4, 1, outfile) || !fwrite(&(meta.height), 4, 1, outfile) || !fwrite(&(meta.BPP), 2, 1, outfile)){
		perror("Could not write to file.\n");
		goto cleanup;
	}

	if (fwrite(out_buf, 1, compressed_size, outfile) != compressed_size){
		perror("Could not write to file.\n");
		goto cleanup;
	}

	goto cleanup;

cleanup:
	if (inp_buf) free(inp_buf);
	if (out_buf) free(out_buf);

	if (infile) fclose(infile);
	if (outfile) fclose(outfile);

	return;
}

int read_meta(FILE* infile, BMP_meta *meta){
	if (!meta) return 0;

	uint8_t sig_buff[2];
	if (fread(&sig_buff, 1, 2, infile) < 2){
		perror("Could not read file.\n");
		return 0;
	}
	if (sig_buff[0] != 'B' || sig_buff[1] != 'M'){
		perror("Invalid signature.\n");
		return 0;
	}
	

	if (fread(&(meta->width), 4, 1, infile) < 1){
		perror("Could not read file.\n");
		return 0;
	}

	if (fread(&(meta->height), 4, 1, infile) < 1){
		perror("Could not read file.\n");
		return 0;
	}

	fseek(infile, 28, SEEK_SET);
	if (fread(&(meta->BPP), 2, 1, infile) < 1){
		perror("Could not read file.\n");
		return 0;
	}
	
	return 1;
}