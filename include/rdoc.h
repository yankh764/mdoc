#ifndef RDOC_H
#define RDOC_H

#include <stdbool.h>

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
int open_doc(char *, char *);

#endif
