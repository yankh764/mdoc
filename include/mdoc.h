#ifndef MDOC_H
#define MDOC_H

#include <stdbool.h>
#include "config.h"

/* To indicate if an previous error eccoured in a functions
   that could overwrite errno with 0 (success) before returning */
extern bool prev_error; 

struct l_list {
	char *obj;
	struct l_list *next;
};

void free_l_list(struct l_list *);
struct l_list *search_for_doc(const char *, const char *, bool, bool);
void display_docs(struct l_list *, bool);
unsigned int count_l_list_nodes(struct l_list *);
char *get_doc_path(const char *, const char *, bool);
char *const *get_open_argv(struct users_configs *);
int open_doc(const char *, const char *);
int sort_docs_alpha(struct l_list *);

#endif
