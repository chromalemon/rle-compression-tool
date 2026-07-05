#include "calc.h"

long calc_file_size(const char* path){
	FILE* fp = fopen(path, "rb");
	if (!fp){
		perror("Could not open file to calculate size.\n");
		return -1L;
	}
	
	if (fseek(fp, 0L, SEEK_END) != 0){
		perror("Fseek failed.\n");
		fclose(fp);
		return -1L;
	}
	long size = ftell(fp);
	if (size == -1L){
		perror("Ftell failed.\n");
	}

	fclose(fp);
	return size;

}

void print_diff(const char* input_path, const char* output_path){
	long original = calc_file_size(input_path);
	long new = calc_file_size(output_path);
	if (original == -1L || new == -1L){
		perror("Could not calculate file sizes.\n");
		return;
	}

	float ratio = ((float)new / original) * 100;

	printf("\nOriginal: %ld", original);
	printf("\nNew: %ld", new);
	printf("\nCompression ratio: %.2f%%\n", ratio);
}
