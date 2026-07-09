#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include "bmp.h"
#include "rle.h"
#include "file.h"

int main(int argc, char* argv[]){
	int opt;
	int res = 0;
	int compress = 0;
	int decompress = 0;
	char format = 0;
	int silence = 0;
	int is_file = 0;
	int block_size = 1;
	int fail = 0;
	int dupe = 0;

	while ((opt = getopt(argc, argv, ":cdbhsr::")) != -1){
		switch (opt){
			case '?':
				fail = 1;
				break;
			case 'c':
				compress = 1;
				break;
			case 'd':
				decompress = 1;
				break;
			case 'b':
				if (format) dupe = 1;
				format = 'B';
				is_file = 1;
				break;
			case 'r':
				if (format) dupe = 1;
				format = 'R';
				is_file = 1;
				if (optarg != 0){
					long temp = strtol(optarg, NULL, 10);
					if (temp < 1 || temp > 8){
						fprintf(stderr, "Error: Invalid word size.\n");
						exit(EXIT_FAILURE);
					}
					block_size = (uint8_t)temp;
				}
				break;
			case 's':
				silence = 1;
				break;
			case 'h':
				break;
			default:
				break;
		}
	}
	if (dupe){
		fprintf(stderr, "Error: Expected one file format, received multiple.\n");
		exit(EXIT_FAILURE);
	}

	if (fail){
		fprintf(stderr, "Error: Unknown argument.\n");
		exit(EXIT_FAILURE);
	}

	if (compress == decompress){
		fprintf(stderr, "Error: Expected one of -c and -d, received multiple.\n");
		exit(EXIT_FAILURE);
	}

	if (format == 0){
		fprintf(stderr, "Error: Expected one input/output format, received none.\n");
		exit(EXIT_FAILURE);
	}

	if (optind + 2 != argc){
		fprintf(stderr, "Error: Expected exactly two additional arguments of input/output paths.\n");
		exit(EXIT_FAILURE);
	}

	char *input_path = argv[optind];
	char *output_path = argv[optind+1];
	

	if (compress){
		if (format == 'B'){
			res = compress_bmp(input_path, output_path);
		} else if (format == 'R'){
			res = compress_regular(input_path, output_path, block_size);
		}
	} else {
		if (format == 'B'){
			res = decompress_bmp(input_path, output_path);
		} else if (format == 'R'){
			res = decompress_regular(input_path, output_path);
		}
	}
	
	if (res != 1){
		exit(EXIT_FAILURE);
	}

	if (!silence && is_file){
		print_diff(input_path, output_path);
	}

	return 0;
	
}
