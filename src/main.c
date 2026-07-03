#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "bmp.h"
#include "rle.h"






int main(int argc, char* argv[]){
	if (argc < 4){
		printf("Usage: %s <-d || -c || -cB || -dB> <input_file> <output_file>\n", argv[0]);
		return 0;
	}
	if (strcmp(argv[1], "-c") == 0){
		printf("Compressing file...\n");
		/*compress_file(argv[2], argv[3]);*/
	} else if (strcmp(argv[1], "-d") == 0){
		printf("Decompressing file...\n");
		/*decompress_file(argv[2], argv[3]);*/
	} else if (strcmp(argv[1], "-cB") == 0){
		printf("Compressing BMP file...\n");
		compress_bmp(argv[2], argv[3]);
	} else if (strcmp(argv[1], "-dB") == 0){
		printf("Decompressing BMP file...\n");
		decompress_bmp(argv[2], argv[3]);
	} else {
		printf("Uknown argument '%s'. Use -c, -d, -cB, or -dB.\n", argv[1]);
	}
	
	
	long prev_size = calc_file_size(argv[2]);
	long curr_size = calc_file_size(argv[3]);
	if (prev_size != -1L && curr_size != -1L){
		printf("Original file size: %ld bytes\n", prev_size);
		printf("New file size: %ld bytes\n", curr_size);
	}
	
	return 0;
}