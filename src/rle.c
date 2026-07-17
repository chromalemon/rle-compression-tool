#include "rle.h"
#include "file.h"

int compress(mem_struct *mem) {
	uint8_t *curr_block = mem->inp_buf;
	uint8_t count = 1;
	uint8_t *prev_block = mem->inp_buf;
	curr_block += mem->block_size;

	while (curr_block < (mem->inp_buf + mem->max_in_size)) {
		if (memcmp(prev_block, curr_block, mem->block_size) == 0) {
			count += 1;
			if (count == 255) {
				if (write_to_buf(mem, count, prev_block) != 1) {
					goto err;
				}
				count = 0;
			}
		} else {
			if (count > 0) {
				if (write_to_buf(mem, count, prev_block) != 1) {
					goto err;
				}
			}
			prev_block = curr_block;
			count = 1;
		}
		curr_block += mem->block_size;
	}
	if (count > 0 && !write_to_buf(mem, count, prev_block)) {
		goto err;
	}

	printf("Compression complete.\n");
	return 1;

err:
	fprintf(stderr, "Error: Could not write to output buffer.\n");
	return 0;
}

int write_to_buf(mem_struct *mem, const uint8_t count,
				 const uint8_t *prev_block) {
	size_t curr_size = mem->out_ptr - mem->out_buf;
	if (curr_size + 1 + mem->block_size > mem->max_out_size) {
		fprintf(stderr, "Error: Output exceeded buffer size.\n");
		return 0;
	}

	*(mem->out_ptr) = count;
	(mem->out_ptr)++;

	memcpy(mem->out_ptr, prev_block, mem->block_size);
	mem->out_ptr += mem->block_size;

	return 1;
}

int decompress(mem_struct *mem) {
	uint8_t *count = mem->inp_buf;
	uint8_t *curr_block = mem->inp_buf + 1;

	while (curr_block < (mem->inp_buf + mem->max_in_size)) {
		for (uint8_t i = 0; i < *count; i++) {
			size_t curr_size = mem->out_ptr - mem->out_buf;
			if (curr_size + mem->block_size > mem->max_out_size) {
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

int compress_regular(const char *input_path, const char *output_path,
					 const uint8_t block_size) {
	int res = 0;

	file_struct file = {0};
	mem_struct mem = {0};

	res = file_init(&file, input_path, output_path);
	if (res != 1) {
		res = 0;
		goto cleanup;
	}
	res = mem_init(&mem, block_size, 0, 1, 0, file.input_size);
	if (res != 1) {
		res = 0;
		goto cleanup;
	}
	if (!fread(mem.inp_buf, mem.max_in_size, 1, file.infile)) {
		fprintf(stderr, "Error: Could not read file into input buffer.\n");
		res = 0;
		goto cleanup;
	}

	res = compress(&mem);
	if (res != 1) {
		res = 0;
		goto cleanup;
	}
	reg_meta meta = {0};
	memcpy(&meta.signature, "RLE!", 4);
	meta.block_size = block_size;
	meta.old_size = mem.max_in_size;
	if (!fwrite(&meta, sizeof(reg_meta), 1, file.outfile)) {
		fprintf(stderr, "Error: Could not write metadata to output file.\n");
		res = 0;
		goto cleanup;
	}

	size_t new_size = mem.out_ptr - mem.out_buf;
	if (!fwrite(mem.out_buf, new_size, 1, file.outfile)) {
		fprintf(stderr, "Error: Could not write payload to file.\n");
		res = 0;
		goto cleanup;
	}

	res = 1;
	goto cleanup;

cleanup:
	if (file.infile)
		fclose(file.infile);
	if (file.outfile)
		fclose(file.outfile);
	if (mem.inp_buf)
		free(mem.inp_buf);
	if (mem.out_buf)
		free(mem.out_buf);
	return res;
}

int decompress_regular(const char *input_path, const char *output_path) {
	int res = 0;

	file_struct file = {0};
	mem_struct mem = {0};

	res = file_init(&file, input_path, output_path);
	if (res != 1) {
		res = 0;
		goto cleanup;
	}
	reg_meta meta = {0};

	if (!fread(&meta, sizeof(reg_meta), 1, file.infile)) {
		fprintf(stderr, "Error: Could not read metadata from input file.\n");
		res = 0;
		goto cleanup;
	}

	if (memcmp("RLE!", &meta.signature, 4) != 0) {
		fprintf(stderr,
				"Error: Input file has not been compressed with this tool.\n");
		res = 0;
		goto cleanup;
	}

	size_t offset = sizeof(reg_meta);

	size_t padded_size =
		((meta.old_size + meta.block_size - 1) / meta.block_size) *
		meta.block_size;

	res = mem_init(&mem, meta.block_size, offset, 0, padded_size,
				   file.input_size);
	if (res != 1) {
		res = 0;
		goto cleanup;
	}

	if (!fread(mem.inp_buf, mem.max_in_size, 1, file.infile)) {
		fprintf(stderr, "Error: Could not read file into input buffer.\n");
		res = 0;
		goto cleanup;
	}

	res = decompress(&mem);
	if (res != 1) {
		res = 0;
		goto cleanup;
	}
	if (!fwrite(mem.out_buf, meta.old_size, 1, file.outfile)) {
		fprintf(stderr, "Error: Could not write payload to file.\n");
		res = 0;
		goto cleanup;
	}

	res = 1;
	goto cleanup;

cleanup:
	if (file.infile)
		fclose(file.infile);
	if (file.outfile)
		fclose(file.outfile);
	if (mem.inp_buf)
		free(mem.inp_buf);
	if (mem.out_buf)
		free(mem.out_buf);
	return res;
}
