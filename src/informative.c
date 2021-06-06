/*
--------------------------------------------------------------
| License: GNU GPL-3.0                                       |
--------------------------------------------------------------
| Unfortunately I couldn't find any efficient way to output  |
| a precise and informative error message without bloating   |
| the whole function with fprintf() to stderr. So I decided  |
| to create this source file which contains functions that   |
| will wrap some known functions from the C standard library |
| therefore if any error occures it'll print a precise and   |
| and informative error message before returning.            |
--------------------------------------------------------------
*/

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include "informative.h"


void *malloc_inf(size_t size) {
	void *ptr;

	if(!(ptr = malloc(size)))
		fprintf(stderr, "%s: can't allocate memory: %s\n", 
				_prog_name_inf_, strerror(errno));

	return ptr;
}


char *fgets_inf(char *str, int size, FILE *stream) {
	char *retval;

	if(!(retval = fgets(str, size, stream)))
		fprintf(stderr, "%s: can't read input\n", _prog_name_inf_);

	return retval;
}


FILE *fopen_inf(const char *pathname, const char *mode) {
	FILE *fp;

	if(!(fp = fopen(pathname, mode)))
		fprintf(stderr, "%s: can't open '%s': %s\n", 
				_prog_name_inf_, pathname, strerror(errno));
	
	return fp;
}


int fclose_inf(FILE *fp) {
	int retval;

	if((retval = fclose(fp)))
		fprintf(stderr, "%s: can't close file: %s\n", 
				_prog_name_inf_, strerror(errno));

	return retval;
}


DIR *opendir_inf(const char *path) {
	DIR *dp;

	if(!(dp = opendir(path)))
		fprintf(stderr, "%s: can't open '%s': %s\n", 
				_prog_name_inf_, path, strerror(errno));

	return dp;
}


int closedir_inf(DIR *dp) {
	int retval;

	if((retval = closedir(dp)))
		fprintf(stderr, "%s: can't close directory: %s\n", 
				_prog_name_inf_, strerror(errno));

	return retval;
}


int stat_inf(const char *pathname, struct stat *statbuf) {
	int retval;

	if((retval = stat(pathname, statbuf)))
		fprintf(stderr, "%s: can't get info on '%s': %s\n", 
				_prog_name_inf_, pathname, strerror(errno));

	return retval;
}


struct dirent *readdir_inf(DIR *dp) {
	struct dirent *entry;

	if(!(entry = readdir(dp)) && errno)
		fprintf(stderr, "%s: can't read file's entry: %s\n", 
				_prog_name_inf_, strerror(errno));

	return entry;
}
