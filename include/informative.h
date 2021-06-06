#ifndef INFORMATIVE_H
#define INFORMATIVE_H

#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>

extern const char *_prog_name_inf_; 

void *malloc_inf(size_t);
char *fgets_inf(char *, int, FILE *);
FILE *fopen_inf(const char *, const char *);
int fclose_inf(FILE *);
DIR *opendir_inf(const char *);
int stat_inf(const char *, struct stat *);
int closedir_inf(DIR *);
struct dirent *readdir_inf(DIR *);

#endif
