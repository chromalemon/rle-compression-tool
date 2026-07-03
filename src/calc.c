
long calc_file_size(const char* path){
	FILE* fp = fopen(path, "rb");
	if (!fp){
		perror("Could not open file.\n");
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