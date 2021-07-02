#ifndef MDOC_H
#define MDOC_H

#include <stdbool.h>
#include "config.h"
#include "informative.h"

/* To indicate if an previous error eccoured in a functions
   that could overwrite errno with 0 (success) before returning */
extern bool prev_error; 

struct l_list {
	char *obj;
	struct l_list *next;
};

struct l_list *search_for_doc(const char *, const char *, bool, bool);
void display_docs(struct l_list *, bool);
char *get_doc_path(const char *, const char *, bool);
char **get_open_doc_argv(char *, char *, char *);
int open_doc(char *const *, const char *);
int sort_docs_alpha(struct l_list *);
void free_l_list(struct l_list *);
unsigned int count_l_list_nodes(struct l_list *);

#endif
