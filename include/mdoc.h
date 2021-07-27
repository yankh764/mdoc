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


void prep_open_doc_argv(char **, const char *, char *, const char *);
int sort_docs_alpha(struct l_list *);
void free_l_list(struct l_list *);
void display_docs(const struct l_list *, const bool);
unsigned int count_l_list_nodes(const struct l_list *);
void reverse_l_list_obj(struct l_list *);
void print_docs_num(const struct l_list *, const bool);
void display_help(const char *);
int open_doc_list(struct users_configs *, const struct l_list *,
				  const bool, const bool);
struct l_list *search_for_doc_multi_dir(const char *, const char *, 
                                        const bool, const bool);

#endif
