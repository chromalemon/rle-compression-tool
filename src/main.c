#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include "bmp.h"
#include "rle.h"
#include "calc.h"

int main(int argc, char* argv[]){
	int opt;
	int compress = 0;
	int decompress = 0;
	char format = 0;
	int silence = 0;
	int is_file = 0;
	
	while ((opt = getopt(argc, argv, "cdbs")) != -1){
		switch (opt){
			case 'c':
				compress = 1;
				break;
			case 'd':
				decompress = 1;
				break;
			case 'b':
				format = 'B';
				is_file = 1;
				break;
			case 's':
				silence = 1;
				break;
			default:
				break;
		}
	}

	if (compress == decompress){
		perror("Cannot have both -c and -d. Choose one.\n");
		exit(EXIT_FAILURE);
	}

	if (format == 0){
		perror("Specify the input/output format, e.g., -b for BMP.\n");
		exit(EXIT_FAILURE);
	}

	if (optind + 2 != argc){
		perror("Expected exactly two additional arguments for input/outputs paths.\n");
		exit(EXIT_FAILURE);
	}

	char *input_path = argv[optind];
	char *output_path = argv[optind+1];
	

	if (compress){
		if (format == 'B'){
			compress_bmp(input_path, output_path);
		}
	} else {
		if (format == 'B'){
			decompress_bmp(input_path, output_path);
		}
	}

	if (!silence && is_file){
		print_diff(input_path, output_path);
	}

	return 0;
	
}
