#include "rle.h"
#include "file.h"
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MIN_BLOCK_SIZE 1
#define MAX_BLOCK_SIZE 8
#define MAX_RUN 255

static int valid_block_size(const uint8_t block_size) {
	return block_size >= MIN_BLOCK_SIZE && block_size <= MAX_BLOCK_SIZE;
}

int compress(mem_struct *mem) {
	if (!valid_block_size(mem->block_size)) {
		(void)fprintf(stderr, "Error: Block size must be between 1 and 8.\n");
		return 0;
	}
	// input must be a whole number of blocks, otherwise the final memcmp
	// would read past the end of the buffer
	if (mem->max_in_size % mem->block_size != 0) {
		(void)fprintf(stderr,
					  "Error: Input size is not a multiple of block size.\n");
		return 0;
	}
	// empty input compresses to empty output
	if (mem->max_in_size == 0) {
		return 1;
	}

	const uint8_t *end = mem->inp_buf + mem->max_in_size;
	uint8_t *prev_block = mem->inp_buf;
	uint8_t *curr_block = mem->inp_buf + mem->block_size;
	uint8_t count = 1;

	while (curr_block < end) {
		// compare running value to new value
		// if equal, increment count
		if (memcmp(prev_block, curr_block, mem->block_size) == 0) {
			count += 1;
			// prevent uint8 overflow: flush a full run and start a new one
			if (count == MAX_RUN) {
				if (write_to_buf(mem, count, prev_block) != 1) {
					goto err;
				}
				count = 0;
			}
		} else {
			// write previous run to buf, set new value and restart count
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
	if (count > 0 && write_to_buf(mem, count, prev_block) != 1) {
		goto err;
	}

	printf("Compression complete.\n");
	return 1;

err:
	(void)fprintf(stderr, "Error: Could not write to output buffer.\n");
	return 0;
}

int write_to_buf(mem_struct *mem, const uint8_t count,
				 const uint8_t *prev_block) {
	size_t curr_size = mem->out_ptr - mem->out_buf;

	// check that output size fits buffer
	if (curr_size + 1 + mem->block_size > mem->max_out_size) {
		(void)fprintf(stderr, "Error: Output exceeded buffer size.\n");
		return 0;
	}

	// write count to buf and increment ptr
	*(mem->out_ptr) = count;
	(mem->out_ptr)++;

	// write data to buf and move ptr by block size
	memcpy(mem->out_ptr, prev_block, mem->block_size);
	mem->out_ptr += mem->block_size;

	return 1;
}

// TODO: Implement PackBits (work in progress saved in wip/)
int decompress(mem_struct *mem) {
	if (!valid_block_size(mem->block_size)) {
		(void)fprintf(stderr, "Error: Block size must be between 1 and 8.\n");
		return 0;
	}

	const size_t pair_size = (size_t)mem->block_size + 1;
	const uint8_t *curr = mem->inp_buf;
	const uint8_t *end = mem->inp_buf + mem->max_in_size;

	while (curr < end) {
		// every run is a count byte followed by one full block
		if ((size_t)(end - curr) < pair_size) {
			(void)fprintf(stderr, "Error: Compressed data is truncated.\n");
			return 0;
		}

		const uint8_t count = *curr;
		const uint8_t *block = curr + 1;

		// the compressor never writes a run of zero
		if (count == 0) {
			(void)fprintf(stderr, "Error: Compressed data is corrupt.\n");
			return 0;
		}

		// write data 'count' times
		for (uint8_t i = 0; i < count; i++) {
			size_t curr_size = mem->out_ptr - mem->out_buf;
			if (curr_size + mem->block_size > mem->max_out_size) {
				(void)fprintf(stderr, "Error: Output exceeded buffer size.\n");
				return 0;
			}

			memcpy(mem->out_ptr, block, mem->block_size);
			mem->out_ptr += mem->block_size;
		}

		curr += pair_size;
	}

	printf("Decompression complete.\n");
	return 1;
}

int compress_regular(const char *input_path, const char *output_path,
					 const uint8_t block_size) {
	int res = 0;

	file_struct file = {0};
	mem_struct mem = {0};

	if (!valid_block_size(block_size)) {
		(void)fprintf(stderr, "Error: Block size must be between 1 and 8.\n");
		return 0;
	}

	res = file_init(&file, input_path, output_path);
	if (res != 1) {
		res = 0;
		goto cleanup;
	}

	// round the input up to a whole number of blocks; the extra bytes are
	// zeroed, and the true size is stored in the header so decompression
	// can trim them off again
	size_t original_size = file.input_size;
	size_t padded_size =
		((original_size + block_size - 1) / block_size) * block_size;

	res = mem_init(&mem, block_size, 0, 0, padded_size);
	if (res != 1) {
		res = 0;
		goto cleanup;
	}
	if (mem.max_in_size > 0) {
		memset(mem.inp_buf, 0, mem.max_in_size);
	}
	if (fread(mem.inp_buf, 1, original_size, file.infile) != original_size) {
		(void)fprintf(stderr,
					  "Error: Could not read file into input buffer.\n");
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
	meta.old_size = original_size;
	if (!fwrite(&meta, sizeof(reg_meta), 1, file.outfile)) {
		(void)fprintf(stderr,
					  "Error: Could not write metadata to output file.\n");
		res = 0;
		goto cleanup;
	}

	size_t new_size = mem.out_ptr - mem.out_buf;
	if (fwrite(mem.out_buf, 1, new_size, file.outfile) != new_size) {
		(void)fprintf(stderr, "Error: Could not write payload to file.\n");
		res = 0;
		goto cleanup;
	}

	res = 1;
	goto cleanup;

cleanup:
	if (file.infile) {
		(void)fclose(file.infile);
	}
	if (file.outfile) {
		(void)fclose(file.outfile);
	}
	if (mem.inp_buf) {
		free(mem.inp_buf);
	}
	if (mem.out_buf) {
		free(mem.out_buf);
	}
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
		(void)fprintf(stderr,
					  "Error: Could not read metadata from input file.\n");
		res = 0;
		goto cleanup;
	}

	if (memcmp("RLE!", &meta.signature, 4) != 0) {
		(void)fprintf(
			stderr,
			"Error: Input file has not been compressed with this tool.\n");
		res = 0;
		goto cleanup;
	}

	// never trust the header: it may have been edited or corrupted
	if (!valid_block_size(meta.block_size)) {
		(void)fprintf(stderr, "Error: Invalid block size in header.\n");
		res = 0;
		goto cleanup;
	}

	size_t offset = sizeof(reg_meta);
	size_t payload_size = file.input_size - offset;
	size_t pair_size = (size_t)meta.block_size + 1;

	// each (count, block) pair expands to at most MAX_RUN blocks, so the
	// original size can't be larger than this
	size_t max_possible =
		(payload_size / pair_size) * MAX_RUN * meta.block_size;
	if (meta.old_size > max_possible) {
		(void)fprintf(stderr,
					  "Error: Header size is larger than the data allows.\n");
		res = 0;
		goto cleanup;
	}

	size_t padded_size =
		((meta.old_size + meta.block_size - 1) / meta.block_size) *
		meta.block_size;

	res = mem_init(&mem, meta.block_size, offset, padded_size, file.input_size);
	if (res != 1) {
		res = 0;
		goto cleanup;
	}

	if (fread(mem.inp_buf, 1, mem.max_in_size, file.infile) !=
		mem.max_in_size) {
		(void)fprintf(stderr,
					  "Error: Could not read file into input buffer.\n");
		res = 0;
		goto cleanup;
	}

	res = decompress(&mem);
	if (res != 1) {
		res = 0;
		goto cleanup;
	}

	// the data must expand to exactly the padded size, otherwise part of
	// the output buffer was never written and must not reach the file
	if ((size_t)(mem.out_ptr - mem.out_buf) != padded_size) {
		(void)fprintf(stderr,
					  "Error: Decompressed size does not match header.\n");
		res = 0;
		goto cleanup;
	}

	if (fwrite(mem.out_buf, 1, meta.old_size, file.outfile) != meta.old_size) {
		(void)fprintf(stderr, "Error: Could not write payload to file.\n");
		res = 0;
		goto cleanup;
	}

	res = 1;
	goto cleanup;

cleanup:
	if (file.infile) {
		(void)fclose(file.infile);
	}
	if (file.outfile) {
		(void)fclose(file.outfile);
	}
	if (mem.inp_buf) {
		free(mem.inp_buf);
	}
	if (mem.out_buf) {
		free(mem.out_buf);
	}
	return res;
}
