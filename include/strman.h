#ifndef STRMAN_H
#define STRMAN_H

#include <stdbool.h>


bool strstr_i(const char *, const char *);
int strsort_alpha(char **, char **, const unsigned int);
unsigned int count_words(const char *);
unsigned int space_to_null(char *);
char *strcpy_dynamic(const char *);
bool alpha_cmp(const char *, const char *);
char *small_let_copy(const char *);
void convert_to_lower(char *);

#endif
