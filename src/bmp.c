#include "rle.h"
#include "bmp.h"
#include "file.h"

int compress_bmp(const char* input_path, const char* output_path){
	int res = 0;
	
	BMP_meta meta;	

	res = read_meta(input_path, &meta);
	if (res != 1) return 0;

	if (meta.compression != 0){
		fprintf(stderr, "Error: BMP file already compressed.\n");
		res = 0;
		goto cleanup;
	}
	
	size_t unpadded_row_length = (meta.width * meta.BPP ) / 8;
	size_t total_padded_row_length = ((unpadded_row_length + 3) / 4) * 4;
	uint8_t padding_bytes_to_skip = total_padded_row_length - unpadded_row_length;
	uint8_t block_size = meta.BPP / 8;

	file_struct file;
	mem_struct mem;

	res = file_init(&file, input_path, output_path);
	if (res != 1) return 0;

	res = mem_init(&mem, input_path, block_size, meta.offset, 1, 0);
	if (res != 1) return 0;
	
	uint8_t *inp_ptr = mem.inp_buf;
	fseek(file.infile, meta.offset, SEEK_SET);
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

	size_t new_size = mem.out_ptr - mem.out_buf;
	if (new_size + 54 > UINT32_MAX){
		fprintf(stderr, "Error: Compressed file too large for BMP3.\n");
		res = 0;
		goto cleanup;
	}

	uint32_t file_size = (uint32_t)(new_size + sizeof(BMP_meta));
	meta.file_size = file_size;
	meta.compression = 1;

	if (!fwrite(&meta, sizeof(BMP_meta), 1, file.outfile)){
		fprintf(stderr, "Error: Could not write BMP metadata to file.\n");
		res = 0;
		goto cleanup;
	}

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

int decompress_bmp(const char* input_path, const char* output_path){
	int res = 0;

	BMP_meta meta;	

	res = read_meta(input_path, &meta);
	if (res != 1) return 0;

	if (meta.compression == 0){
		fprintf(stderr, "Error: BMP file is not compressed.\n");
		res = 0;
		goto cleanup;
	}

	size_t unpadded_row_length = (meta.width * meta.BPP) / 8;
	size_t total_padded_row_length = ((unpadded_row_length + 3) / 4) * 4;
	uint8_t padding_bytes_to_skip = total_padded_row_length - unpadded_row_length;
	size_t old_size = total_padded_row_length * meta.height;
	uint8_t block_size = meta.BPP / 8;
	uint8_t zero = 0;	

	file_struct file;
	mem_struct mem;

	res = file_init(&file, input_path, output_path);
	if (res != 1) return 0;

	res = mem_init(&mem, input_path, block_size, meta.offset, 0, old_size);
	if (res != 1) return 0;

	fseek(file.infile, meta.offset, SEEK_SET);
	if (!fread(mem.inp_buf, mem.max_in_size, 1, file.infile)){
		fprintf(stderr, "Error: Could not read file into input buffer.\n");
		res = 0;
		goto cleanup;
	}

	res = decompress(&mem);
	if (res != 1) goto cleanup;

	size_t new_size = mem.out_ptr - mem.out_buf;
	uint32_t file_size = (uint32_t)(new_size + sizeof(BMP_meta));
	meta.file_size = file_size;
	meta.compression = 0;

	if (!fwrite(&meta, sizeof(BMP_meta), 1, file.outfile)){
		fprintf(stderr, "Error: Could not write BMP metadata to file.\n");
		res = 0;
		goto cleanup;
	}

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

int read_meta(const char* path, BMP_meta *meta){
	if (!meta){
		fprintf(stderr, "Error: No meta pointer provided to read_meta.\n");
		return 0;
	}

	FILE *infile = fopen(path, "rb");
	if (!infile){
		fprintf(stderr, "Error: Could not open input file.\n");
		return 0;
	} 
	int res = 0;

	uint8_t sig_buf[2];
	fseek(infile, 0, SEEK_SET);
	if (fread(&sig_buf, 1, 2, infile) < 2){
		fprintf(stderr, "Error: Could not read file.\n");
		goto cleanup;
	}

	if (memcmp("BM", sig_buf, 2) != 0){
		fprintf(stderr, "Error: Invalid signature, file must be BMP V3.\n");
		goto cleanup;
	}
	
	fseek(infile, 0, SEEK_SET);
	if (!fread(meta, sizeof(BMP_meta), 1, infile)){
		fprintf(stderr, "Error: Could not parse BMP header.\n");
		goto cleanup;
	}

	if (meta->DIB_size != 40 || meta->offset != 54 || meta->BPP != 24){
		fprintf(stderr, "Error: BMP file must be V3, 24 BPP.\n");
		goto cleanup;
	}

	res = 1;
	goto cleanup;

cleanup:
	if (infile) fclose(infile);
	return res;
}
