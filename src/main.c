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

	while ((opt = getopt(argc, argv, "cdb")) != -1){
		switch (opt){
			case 'c':
				compress = 1;
				break;
			case 'd':
				decompress = 1;
				break;
			case 'b':
				format = 'B';
				break;
			default:
				break;
		}
	}

	if (compress == decompress){
		exit(EXIT_FAILURE);
	}

	if (format == 0){
		exit(EXIT_FAILURE);
	}

	if (optind + 2 != argc){
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

	return 0;
	
}
