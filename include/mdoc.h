#ifndef MDOC_H
#define MDOC_H

#include <stdbool.h>
#include <sys/stat.h>
#include "config.h"

/* To indicate if an previous error eccoured in a functions
   that could overwrite errno with 0 (success) before returning */
extern bool prev_error; 


struct doc_list {
	char *path;
	char *name;
	struct stat *stbuf; 
	struct doc_list *next;
};


int sort_docs_alpha(struct doc_list *);
void free_doc_list(struct doc_list *);
void display_docs_names(const struct doc_list *, bool);
unsigned int count_doc_list_nodes(const struct doc_list *);
void reverse_doc_list(struct doc_list *);
void print_docs_num(const struct doc_list *, bool);
void display_help(const char *);
struct doc_list *search_for_doc_multi_dir(const char *, const char *, bool, bool);
int open_doc_path(const struct users_configs *, const char *);
void print_opening_doc(const char *, bool);
int print_docs_details(const struct doc_list *, bool);

#endif
