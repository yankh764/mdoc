#ifndef INFORMATIVE_H
#define INFORMATIVE_H

#include <stdio.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>

extern const char *prog_name_inf; 


void *malloc_inf(size_t);
char *fgets_inf(char *, int, FILE *);
FILE *fopen_inf(const char *, const char *);
int fclose_inf(FILE *);
DIR *opendir_inf(const char *);
int stat_inf(const char *, struct stat *);
int closedir_inf(DIR *);
struct dirent *readdir_inf(DIR *);
pid_t fork_inf(void);
pid_t waitpid_inf(pid_t, int *, int);
int execv_inf(const char *, char *const *);
void *realloc_inf(void *, size_t);
int execvp_inf(const char *, char *const *);
void *reallocarray_inf(void *, size_t, size_t);
char *getenv_inf(const char *);

#endif
